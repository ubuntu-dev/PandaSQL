#ifndef PANDASQL_STATEMENT_H
#define PANDASQL_STATEMENT_H

#include "Catalog/Table.h"

#include "Utils/Join.h"
#include "Utils/Predicate.h"

namespace PandaSQL
{

class PandaDB;
class BooleanExpr;

class Statement
{
public:
	Statement(PandaDB *io_pDB);
	~Statement();

	enum StatementType
	{
		kStmtUnknown = 0,
		kStmtSelect = 1,
		kStmtUpdate = 2,
		kStmtInsert = 3,
		kStmtDelete = 4,
		kStmtCreateTable = 5,
		kStmtCreateIndex = 6,
		kStmtDropTable = 7,
		kStmtDropIndex = 8,
	};

	void Clear();

	void SetOriginalStmtText(const std::string inStmtText);

	void SetStatementType(StatementType inStmtType) { mStmtType = inStmtType; }
	StatementType GetStatementType() const { return mStmtType; }

	//For create_table_stmt, created table[1]
	//For create_index_stmt, affected table[1]
	//For drop_table_stmt, affected table[1]
	//For update_stmt, affected table[1]
	//For insert_stmt, affected table[1]
	//For delete_stmt, affected table[1]
	//For select_stmt, affected table[1..N]
	void AddTableRef(const std::string &inTableRef);

	//For update_stmt, applied value[1..N]
	//For insert_stmt, applied value[1..N]
	void AddExprRef(const Expr &inExpr);

	//For create_index_stmt, indexed column[1]
	//For select_stmt, selected column[1..N]
	//For insert_stmt, affected column[1..N]
	//For update_stmt, affected column[1..N]
	void AddColumnDefWithName(const ColumnQualifiedName &inQualifiedName);

	//For cerate_table_stmt, column def[1..N]
	void AddColumnDef(const ColumnDef &inDef);
	void AddAllColumns();

	//For create_index_stmt
	void SetIndexRef(const std::string &inIndexRef);

	void SetPredicate(const Predicate &inPredicate);

	//Rewrite statment. e.g. Translate to fully qualified column name
	Status Prepare();

	Status Execute(bool loadTable);
	void PrintStatement();

	void SetWhereClauseExpression(const BooleanExpr *inWhereExpr);

private:
	PandaDB *mpDB;

	std::string	mOrigStmtText;

	StatementType mStmtType;
	Table::TableRefList mTableRefs;

	ExprList mSetExprList;

	ColumnDefList mColumnDefs;
	bool	mAllColumns;

	JoinList mJoinList;

	Predicate mPredicate;

	std::string mIndexRef;

	const BooleanExpr *mpWhereExpr;
};


}	// PandaSQL


#endif //PANDASQL_STATEMENT_H
