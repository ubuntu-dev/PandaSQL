#include "stdafx.h"

#include "Storage/BambooDB/BambooBackend.h"

#include "BambooDB/DB.h"

#include "Utils/Debug.h"
#include "Utils/Common.h"

namespace PandaSQL
{

static const char *const kDBName = "mydb.panda";

BambooBackend::BambooBackend(const std::string &inRootPath)
:
IDBBackend(inRootPath)
,mpDBHandle(NULL)
,mIsOpen(false)
{
	BambooDB::DB dbHandle(inRootPath);
}

BambooBackend::~BambooBackend()
{

}

Status BambooBackend::Open()
{
	Status result;
	PDASSERT(!mIsOpen);

	if (!mIsOpen)
	{
		////Set DB_AUTO_COMMIT to the environment, so all operation would be transactional protected
		////If set on DB, seems like it would causing crash if adding index for that DB
		//int ret = mpDBEnv->set_flags(mpDBEnv, DB_AUTO_COMMIT, 1);

		//PDASSERT(ret == 0);

		//ret = mpDBEnv->open(mpDBEnv
		//	, mRootPath.c_str()
		//	, DB_CREATE | DB_INIT_LOG | DB_INIT_LOCK | DB_INIT_MPOOL | DB_INIT_TXN
		//	  | DB_PRIVATE // restric to single process
		//	, 0);

		//if (ret != 0)
		//{
		//	PDDebugOutputVerbose(db_strerror(ret));
		//}

		//PDASSERT(ret == 0);

		mIsOpen = true;
	}

	return result;
}	

Status BambooBackend::Close()
{
	Status result;

	PDASSERT(mIsOpen);

	if (mIsOpen)
	{
		//int ret = mpDBEnv->close(mpDBEnv, 0);
		//mpDBEnv = NULL;

		//PDASSERT(ret == 0);

		mIsOpen = false;
	}

	return result;
}

Status BambooBackend::OpenTable(const std::string &tableName, OpenMode openMode, PayloadPtr *io_payload)
{
	Status result;

	//DB *pTable = NULL;

	//int ret = db_create(&pTable, mpDBEnv, 0);

	//PDASSERT(ret == 0);

	//u_int32_t flags = 0;

	//if (openMode & kCreate)
	//{
	//	flags |= DB_CREATE;
	//	
	//	// It would return error if table already exists
	//	if (openMode & kErrorIfExists)
	//	{
	//		flags |= DB_EXCL;
	//	}
	//}

	//ret = pTable->open(pTable,
	//	NULL
	//	, kDBName
	//	, tableName.c_str()
	//	, DB_RECNO
	//	, flags
	//	, 0);


	//if (ret != 0)
	//{
	//	PDDebugOutputVerbose(db_strerror(ret));

	//	if (ret == EEXIST)
	//	{
	//		result = Status::kTableAlreadyExists;
	//	}
	//	else
	//	{
	//		result = Status::kInternalError;
	//	}

	//	ret = pTable->close(pTable, 0);

	//	//Assume we can close it after failling open the db
	//	PDASSERT(ret == 0);
	//}

	//if (result.OK())
	//{
	//	*io_payload = (PayloadPtr)pTable;
	//}
	//else
	//{
	//	*io_payload = NULL;
	//}

	return result;
}

Status BambooBackend::DropTable(const std::string &tableName, PayloadPtr payload)
{
	Status result;

	//DB *pTable = (DB *)payload;

	////First close the table
	//int ret = pTable->close(pTable, 0);

	//if (ret != 0)
	//{
	//	PDDebugOutputVerbose(db_strerror(ret));
	//	result = Status::kInternalError;
	//}

	//if (result.OK())
	//{
	//	ret = mpDBEnv->dbremove(mpDBEnv
	//		, NULL
	//		, kDBName
	//		, tableName.c_str()
	//		, 0);

	//	if (ret != 0)
	//	{
	//		PDDebugOutputVerbose(db_strerror(ret));

	//		if (ret == ENOENT)
	//		{
	//			result = Status::kTableMissing;
	//		}
	//		else
	//		{
	//			result = Status::kInternalError;
	//		}
	//	}
	//}

	return result;
}

Status BambooBackend::OpenIndex(const std::string &indexName, const std::string &tableName, const TupleDesc &tupleDesc, const UInt32List &tupleIndexList, bool isUnique, OpenMode openMode, PayloadPtr tablePayload, PayloadPtr *io_indexPayload)
{
	Status result;

	//DB *pTable = (DB *)tablePayload;

	//if (result.OK())
	//{
	//	DB *pIndex = NULL;
	//	int ret;

	//	ret = db_create(&pIndex, mpDBEnv, 0);

	//	PDASSERT(ret == 0);

	//	u_int32_t flags = 0;

	//	if (!isUnique)
	//	{
	//		flags |= DB_DUPSORT;
	//	}

	//	ret = pIndex->set_flags(pIndex, flags);

	//	PDASSERT(ret == 0);

	//	u_int32_t openFlags = 0;
	//	
	//	if (openMode & kCreate)
	//	{
	//		openFlags |= DB_CREATE;

	//		// It would return error if table already exists
	//		if (openMode & kErrorIfExists)
	//		{
	//			openFlags |= DB_EXCL;
	//		}
	//	}

	//	ret = pIndex->open(pIndex
	//		, NULL
	//		, kDBName
	//		, GetFullIndexName(indexName, tableName).c_str()
	//		, DB_BTREE
	//		, openFlags
	//		, 0);

	//	if (ret != 0)
	//	{
	//		PDDebugOutputVerbose(db_strerror(ret));

	//		if (ret == EEXIST)
	//		{
	//			result = Status::kIndexAlreadyExists;
	//		}
	//		else
	//		{
	//			result = Status::kInternalError;
	//		}

	//		ret = pIndex->close(pIndex, 0);

	//		//Assume we can close it after failling creating the index
	//		PDASSERT(ret == 0);
	//	}

	//	if (result.OK())
	//	{
	//		IndexMapEntry indexEntry;

	//		IndexInfo indexInfo;
	//		indexInfo.tupleDesc = tupleDesc;
	//		indexInfo.tupleIndexList = tupleIndexList;

	//		mIndexMap[pIndex] = indexInfo;

	//		pIndex->app_private = (void *)&mIndexMap[pIndex];

	//		ret = pTable->associate(pTable, NULL, pIndex, IndexBinder, 0);

	//		if (ret != 0)
	//		{
	//			result = Status::kInternalError;
	//		}
	//	}

	//	if (result.OK())
	//	{
	//		*io_indexPayload = (PayloadPtr)pIndex;
	//	}
	//	else
	//	{
	//		*io_indexPayload = NULL;
	//	}
	//}

	return result;
}

Status BambooBackend::DropIndex(const std::string &indexName, const std::string &tableName, PayloadPtr indexPayload)
{
	Status result;

	//DB *pIndex = (DB *)indexPayload;

	//IndexMap::iterator iter = mIndexMap.find(pIndex);

	//if (iter != mIndexMap.end())
	//{
	//	mIndexMap.erase(iter);
	//}

	//int ret = pIndex->close(pIndex, 0);

	//if (ret != 0)
	//{
	//	PDDebugOutputVerbose(db_strerror(ret));
	//	result = Status::kInternalError;
	//}

	//if (result.OK())
	//{
	//	int ret = mpDBEnv->dbremove(mpDBEnv
	//		, NULL
	//		, kDBName
	//		, GetFullIndexName(indexName, tableName).c_str()
	//		, 0);

	//	if (ret != 0)
	//	{
	//		PDDebugOutputVerbose(db_strerror(ret));

	//		if (ret == ENOENT)
	//		{
	//			result = Status::kIndexMissing;
	//		}
	//		else
	//		{
	//			result = Status::kInternalError;
	//		}
	//	}
	//}

	return result;
}

Status BambooBackend::InsertData(const std::string &tableName, const TupleDesc &tupleDesc, const ValueList &tupleValueList, PayloadPtr tablePayload)
{
	Status result;

	//DB *pTable = (DB *)tablePayload;

	//DBT key;
	//DBT data;
	//int ret;

	//db_recno_t recno;

	//memset(&key, 0, sizeof(key));
	//memset(&data, 0, sizeof(data));

	//std::string rowString;
	//TupleToString(tupleDesc, tupleValueList, &rowString);
	//data.data = (void *)rowString.c_str();
	//data.size = (u_int32_t)rowString.length();

	//ret = pTable->put(pTable, NULL, &key, &data, DB_APPEND);

	//if (ret != 0)
	//{
	//	PDDebugOutputVerbose(db_strerror(ret));
	//}
	//else
	//{
	//	recno = *(db_recno_t *)(key.data);
	//	printf("new record number is %u\n", recno);
	//}

	return result;
}

TupleIterator* BambooBackend::CreateSeqScanIterator(const std::string &tableName, const TupleDesc &tupleDesc, const Expr *inTuplePredicate, PayloadPtr payload)
{
	TupleIterator *result = NULL;

	//DB *pTable = (DB *)payload;
	//
	//result = new BDBSeqScanIterator(tupleDesc, pTable, mpDBEnv);

	return result;
}

TupleIterator* BambooBackend::CreateIndexScanIterator(const std::string &indexName, const UInt32List &columnIndexList, const TupleDesc &tupleDesc, const Expr *inTuplePredicate, PayloadPtr payload)
{
	TupleIterator *result = NULL;

	//DB *pIndex = (DB *)payload;
	//
	//result = new BDBIndexScanIterator(columnIndexList, tupleDesc, pIndex, mpDBEnv);

	return result;
}

}	// PandaSQL