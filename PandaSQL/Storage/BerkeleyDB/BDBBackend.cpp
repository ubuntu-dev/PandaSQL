#include "stdafx.h"

#include "Storage/BerkeleyDB/BDBBackend.h"

#include "Storage/BerkeleyDB/BDBScanIterator.h"
#include "Storage/BerkeleyDB/BDBTypes.h"
#include "Storage/BerkeleyDB/Transaction/BDBTransaction.h"

#include "Utils/Debug.h"
#include "Utils/Common.h"

namespace PandaSQL
{

static const char *const kDBName = "mydb.panda";

bool BDBBackend::IndexEntryKey::operator==(const BDBBackend::IndexEntryKey &rhs) const
{
	return this->indexName == rhs.indexName
		&& this->tableName == rhs.tableName
		;
}

bool BDBBackend::IndexEntryKey::operator<(const BDBBackend::IndexEntryKey &rhs) const
{
	if (this->tableName == rhs.tableName)
	{
		return this->indexName < rhs.indexName;
	}

	return this->tableName < rhs.tableName;
}

BDBBackend::BDBBackend(const std::string &inRootPath)
:
IDBBackend(inRootPath)
,mpDBEnv(NULL)
,mIsOpen(false)
{
	int ret;

	/*
	* Create an environment and initialize it for additional error
	* reporting.
	*/
	if ((ret = db_env_create(&mpDBEnv, 0)) != 0)
	{
		PDDebugOutputVerbose(db_strerror(ret));
	}
}

BDBBackend::~BDBBackend()
{

}

Status BDBBackend::Open()
{
	Status result;

	PDASSERT(mpDBEnv);
	PDASSERT(!mIsOpen);

	if (!mIsOpen)
	{
		//Set DB_AUTO_COMMIT to the environment, so all operation would be transactional protected
		//If set on DB, seems like it would causing crash if adding index for that DB
		int ret = mpDBEnv->set_flags(mpDBEnv, DB_AUTO_COMMIT, 1);

		PDASSERT(ret == 0);

		ret = mpDBEnv->open(mpDBEnv
			, mRootPath.c_str()
			, DB_CREATE | DB_INIT_LOG | DB_INIT_LOCK | DB_INIT_MPOOL | DB_INIT_TXN
			  | DB_PRIVATE // restric to single process
			, 0);

		if (ret != 0)
		{
			PDDebugOutputVerbose(db_strerror(ret));
		}

		PDASSERT(ret == 0);

		mIsOpen = true;
	}

	return result;
}	

Status BDBBackend::Close()
{
	Status result;

	PDASSERT(mIsOpen);

	if (mIsOpen)
	{
		int ret = mpDBEnv->close(mpDBEnv, 0);
		mpDBEnv = NULL;

		PDASSERT(ret == 0);

		mIsOpen = false;
	}

	return result;
}

Status BDBBackend::OpenTable(const std::string &inTableName, OpenMode openMode)
{
	PDASSERT(mTableMap.find(inTableName) == mTableMap.end());

	Status result;

	DB *pTable = NULL;

	int ret = db_create(&pTable, mpDBEnv, 0);

	PDASSERT(ret == 0);

	u_int32_t flags = 0;

	if (openMode & kCreate)
	{
		flags |= DB_CREATE;
		
		// It would return error if table already exists
		if (openMode & kErrorIfExists)
		{
			flags |= DB_EXCL;
		}
	}

	ret = pTable->open(pTable,
		NULL
		, kDBName
		, inTableName.c_str()
		, DB_RECNO
		, flags
		, 0);


	if (ret != 0)
	{
		PDDebugOutputVerbose(db_strerror(ret));

		if (ret == EEXIST)
		{
			result = Status::kTableAlreadyExists;
		}
		else
		{
			result = Status::kInternalError;
		}

		ret = pTable->close(pTable, 0);

		//Assume we can close it after failling open the db
		PDASSERT(ret == 0);
	}

	if (result.OK())
	{
		TableMapEntry tableEntry(inTableName.c_str(), pTable);
		mTableMap.insert(tableEntry);
	}

	return result;
}

Status BDBBackend::CloseTable(const std::string &inTableName)
{
	Status result;

	DB *pTable = NULL;

	result = this->GetTableByName_Private(inTableName, &pTable);

	if (result.OK())
	{
		TableMap::iterator iter = mTableMap.find(inTableName);

		if (iter != mTableMap.end())
		{
			mTableMap.erase(iter);
		}

		int ret = pTable->close(pTable, 0);

		if (ret != 0)
		{
			PDDebugOutputVerbose(db_strerror(ret));
			result = Status::kInternalError;
		}
	}

	return result;
}

Status BDBBackend::DropTable(const std::string &tableName)
{
	Status result;

	int ret = mpDBEnv->dbremove(mpDBEnv
		, NULL
		, kDBName
		, tableName.c_str()
		, 0);

	if (ret != 0)
	{
		PDDebugOutputVerbose(db_strerror(ret));

		if (ret == ENOENT)
		{
			result = Status::kTableMissing;
		}
		else
		{
			result = Status::kInternalError;
		}
	}

	return result;
}

static int IndexBinder(DB *secondary, const DBT *pkey, const DBT *pdata, DBT *skey)
{
	memset(skey, 0, sizeof(DBT));

	const BDBBackend::IndexInfo *indexInfo = (const BDBBackend::IndexInfo *)secondary->app_private;

	std::string rowString;	
	rowString.append((const char *)pdata->data, pdata->size);

	uint32_t offset;
	uint32_t length;

	StringToTupleElmentOffsetAndLength(indexInfo->tupleDesc, rowString, indexInfo->indexList[0], &offset, &length); 
	
	skey->data = (char *)pdata->data + offset;
	skey->size = length;

	std::string keyStr;
	keyStr.append((const char *)skey->data, skey->size);

	db_recno_t recno = *(db_recno_t *)(pkey->data);

	std::cout << "Index Key-Value: " << keyStr << ", " << recno << std::endl;

	return 0;
}

Status BDBBackend::OpenIndex(const std::string &indexName, const std::string &tableName, const TupleDesc &tupleDesc, const std::vector<int32_t> &indexList, bool isUnique, OpenMode openMode)
{
	Status result;

	DB *pTable = NULL;
	
	result = this->GetTableByName_Private(tableName, &pTable);

	if (result.OK())
	{
		DB *pIndex = NULL;
		int ret;

		ret = db_create(&pIndex, mpDBEnv, 0);

		PDASSERT(ret == 0);

		u_int32_t flags = 0;

		if (!isUnique)
		{
			flags |= DB_DUPSORT;
		}

		ret = pIndex->set_flags(pIndex, flags);

		PDASSERT(ret == 0);

		u_int32_t openFlags = 0;
		
		if (openMode & kCreate)
		{
			openFlags |= DB_CREATE;

			// It would return error if table already exists
			if (openMode & kErrorIfExists)
			{
				openFlags |= DB_EXCL;
			}
		}

		ret = pIndex->open(pIndex
			, NULL
			, kDBName
			, GetFullIndexName(indexName, tableName).c_str()
			, DB_BTREE
			, openFlags
			, 0);

		if (ret != 0)
		{
			PDDebugOutputVerbose(db_strerror(ret));

			if (ret == EEXIST)
			{
				result = Status::kIndexAlreadyExists;
			}
			else
			{
				result = Status::kInternalError;
			}

			ret = pIndex->close(pIndex, 0);

			//Assume we can close it after failling creating the index
			PDASSERT(ret == 0);
		}

		if (result.OK())
		{
			IndexEntryKey entryKey;
			entryKey.indexName = indexName;
			entryKey.tableName = tableName;

			IndexInfo indexInfo;
			indexInfo.indexDB = pIndex;
			indexInfo.tupleDesc = tupleDesc;
			indexInfo.indexList = indexList;

			mIndexMap[entryKey] = indexInfo;

			pIndex->app_private = (void *)&mIndexMap[entryKey];

			ret = pTable->associate(pTable, NULL, pIndex, IndexBinder, 0);

			if (ret != 0)
			{
				result = Status::kInternalError;
			}
		}
       

	}

	return result;
}

Status BDBBackend::DropIndex(const std::string &indexName, const std::string &tableName)
{
	Status result;

	DB *pIndex = NULL;

	result = this->GetIndexByName_Private(indexName, tableName, &pIndex);

	if (result.OK())
	{
		IndexEntryKey entryKey;
		entryKey.indexName = indexName;
		entryKey.tableName = tableName;

		IndexMap::iterator iter = mIndexMap.find(entryKey);

		if (iter != mIndexMap.end())
		{
			mIndexMap.erase(iter);
		}

		int ret = pIndex->close(pIndex, 0);

		if (ret != 0)
		{
			PDDebugOutputVerbose(db_strerror(ret));
			result = Status::kInternalError;
		}

		if (result.OK())
		{
			int ret = mpDBEnv->dbremove(mpDBEnv
				, NULL
				, kDBName
				, GetFullIndexName(indexName, tableName).c_str()
				, 0);

			if (ret != 0)
			{
				PDDebugOutputVerbose(db_strerror(ret));

				if (ret == ENOENT)
				{
					result = Status::kIndexMissing;
				}
				else
				{
					result = Status::kInternalError;
				}
			}
		}
	}

	return result;
}

Status BDBBackend::InsertData(const std::string &tableName, const TupleDesc &tupleDesc, const ValueList &tupleValueList)
{
	Status result;

	DB *pTable = NULL;
	
	result = this->GetTableByName_Private(tableName, &pTable);

	if (result.OK())
	{
		DBT key;
		DBT data;
		int ret;

		db_recno_t recno;
		
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));

		std::string rowString;
		TupleToString(tupleDesc, tupleValueList, &rowString);
		data.data = (void *)rowString.c_str();
		data.size = rowString.length();

		ret = pTable->put(pTable, NULL, &key, &data, DB_APPEND);

		if (ret != 0)
		{
			PDDebugOutputVerbose(db_strerror(ret));
		}
		else
		{
			recno = *(db_recno_t *)(key.data);
			printf("new record number is %u\n", recno);
		}
	}

	return result;
}

TupleIterator* BDBBackend::CreateScanIterator(const std::string &tableName, const TupleDesc &tupleDesc, const TuplePredicate *inTuplePredicate /*= NULL*/)
{
	TupleIterator *result = NULL;

	DB *pTable = NULL;
	
	Status localResult = this->GetTableByName_Private(tableName, &pTable);

	if (localResult.OK())
	{
		result = new BDBScanIterator(tupleDesc, pTable, mpDBEnv);
	}

	return result;
}

Status BDBBackend::GetTableByName_Private(const std::string &name, DB **o_table) const
{
	Status result;

	BDBBackend::TableMap::const_iterator iter = mTableMap.find(name);

	if (iter != mTableMap.end())
	{
		*o_table = iter->second;
	}
	else
	{
		//The caller should always get a valid table.
		//kTableMissing error should be returned by higher layer
		PDASSERT(0);

		*o_table = NULL;

		result = Status::kInternalError;
	}

	return result;
}

Status BDBBackend::GetIndexByName_Private(const std::string &indexName, const std::string &tableName, DB **o_index) const
{	
	Status result;

	IndexEntryKey entryKey;
	entryKey.indexName = indexName;
	entryKey.tableName = tableName;

	BDBBackend::IndexMap::const_iterator iter = mIndexMap.find(entryKey);

	if (iter != mIndexMap.end())
	{
		*o_index = iter->second.indexDB;
	}
	else
	{
		//The caller should always get a valid index.
		//kTableMissing error should be returned by higher layer
		PDASSERT(0);

		*o_index = NULL;

		result = Status::kInternalError;
	}

	return result;
}

}	// PandaSQL