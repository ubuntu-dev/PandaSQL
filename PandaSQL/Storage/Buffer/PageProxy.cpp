#include "stdafx.h"

#include "Storage/Buffer/PageProxy.h"

#include "Storage/Buffer/BufMgr.h"
#include "Storage/Buffer/Page.h"

namespace PandaSQL
{

static const uint32_t kBufCount = 256;

PageProxy::PageProxy(uint32_t inPageSize, File *io_pagedFile)
:
mPageSize(inPageSize)
,mpBufMgr(new BufMgr(kBufCount, inPageSize, io_pagedFile))
{
}

PageProxy::~PageProxy()
{
	delete mpBufMgr;
	mpBufMgr = NULL;
}

//Status PageProxy::ReadPage(uint32_t inPageNum, File::Offset inPageOffset, File::Size amount, void *o_buf, File::Size *o_bytesRead)
//{
//	return mpPagedFile->Read(inPageNum * mPageSize + inPageOffset, amount, o_buf, o_bytesRead);
//}
//
//Status PageProxy::WritePage(uint32_t inPageNum, File::Offset inPageOffset, File::Size amount, const void *inBuf, File::Size *o_bytesWritten)
//{
//	return mpPagedFile->Write(inPageNum * mPageSize + inPageOffset, amount, inBuf, o_bytesWritten);
//}

//Status PageProxy::Flush()
//{
//	return mpPagedFile->Flush();
//}

Status PageProxy::GetPage(PageNum inPageNum, AccessFlags inAccessMode, char **o_pageData)
{
	Status result;

	Page thePage;

	result = mpBufMgr->PinPage(inPageNum, &thePage);

	if (result.OK())
	{
		*o_pageData = thePage.mPageData;
	}

	return result;
}

Status PageProxy::PutPage(PageNum inPageNum, bool inDirty, const char *inPageData)
{
	Status result;

	result = mpBufMgr->UnpinPage(inPageNum, inDirty);

	return result;
}

Status PageProxy::NewPage(PageNum *o_pageNum)
{
	Status result;

	PageNum pageNum;

	result = mpBufMgr->NewPage(&pageNum);

	if (result.OK())
	{
		*o_pageNum = pageNum;
	}

	return result;
}

Status PageProxy::GetPageCount(PageNum *o_pageCount) const
{
	Status result;

	result = mpBufMgr->GetTotalPages(o_pageCount);

	return result;
}

uint32_t PageProxy::GetPageSize() const
{
	return mPageSize;
}

}	// PandaSQL
