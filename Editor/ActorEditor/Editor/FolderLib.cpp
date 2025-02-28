#include "stdafx.h"
#pragma hdrstop

#include "FolderLib.h"
#include "xr_trims.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

static TElTreeItem*			DragItem;
static FOLDER::TOnAfterDrag	DragAction;

void FOLDER::ShowPPMenu(TMxPopupMenu* M, TExtBtn* B){
    POINT pt;
    GetCursorPos(&pt);
	M->Popup(pt.x,pt.y-10);
    if (B) B->MouseManualUp();
}
//---------------------------------------------------------------------------

LPCSTR FOLDER::GetFolderName(const AnsiString& full_name, AnsiString& dest)
{
    for (int i=full_name.Length(); i>=1; i--)
    	if (full_name[i]=='\\'){
        	dest=full_name.SubString(1,i);
            break;
        }
    return dest.c_str();
}

LPCSTR FOLDER::GetObjectName(const AnsiString& full_name, AnsiString& dest)
{
    for (int i=full_name.Length(); i>=1; i--)
    	if (full_name[i]=='\\'){
        	dest=full_name.SubString(i+1,full_name.Length());
            break;
        }
    return dest.c_str();
}

// �������� ��� �� ���������� ����� �� ���������
// ����� �������� ���� �� �������� ��� �������
bool FOLDER::MakeName(TElTreeItem* begin_item, TElTreeItem* end_item, AnsiString& name, bool bOnlyFolder)
{
	if (begin_item){
    	TElTreeItem* node = (DWORD(begin_item->Data)==TYPE_OBJECT)?begin_item->Parent:begin_item;
        name = "";
        while (node){
			name.Insert(node->Text+AnsiString('\\'),0);
        	if (node==end_item) break;
            node=node->Parent;
        }
        if (!bOnlyFolder){
        	if (DWORD(begin_item->Data)==TYPE_OBJECT) name+=begin_item->Text;
            else return false;
        }
        return true;
    }else{
		name = "";
        return false;
    }
}
//---------------------------------------------------------------------------

bool FOLDER::MakeFullName(TElTreeItem* begin_item, TElTreeItem* end_item, AnsiString& name)
{
	if (begin_item){
    	TElTreeItem* node = begin_item;
        name = node->Text;
		node = node->Parent;
        while (node){
			name.Insert(node->Text+AnsiString('\\'),0);
        	if (node==end_item) break;
            node=node->Parent;
        }
        return true;
    }else{
		name = "";
        return false;
    }
}
//---------------------------------------------------------------------------

TElTreeItem* FOLDER::FindItemInFolder(DWORD type, TElTree* tv, TElTreeItem* start_folder, const AnsiString& name, bool bIgnoreExt)
{
	if (bIgnoreExt){
        if (start_folder){
            for (TElTreeItem* node=start_folder->GetFirstChild(); node; node=start_folder->GetNextChild(node)){
                AnsiString nm = ChangeFileExt(node->Text,"");
                if (type==((DWORD)(node->Data))&&(nm==name)) return node;
            }
        }else{
            for (TElTreeItem* node=tv->Items->GetFirstNode(); node; node=node->GetNextSibling()){
                AnsiString nm = ChangeFileExt(node->Text,"");
                if (type==((DWORD)(node->Data))&&(nm==name)) return node;
            }
        }
    }else{
       if (start_folder){
            for (TElTreeItem* node=start_folder->GetFirstChild(); node; node=start_folder->GetNextChild(node))
                if (type==((DWORD)(node->Data))&&(node->Text==name)) return node;
        }else{
            for (TElTreeItem* node=tv->Items->GetFirstNode(); node; node=node->GetNextSibling())
                if (type==((DWORD)(node->Data))&&(node->Text==name)) return node;
        }
    }

    return 0;
}
//---------------------------------------------------------------------------

TElTreeItem* FOLDER::FindItemInFolder(TElTree* tv, TElTreeItem* start_folder, const AnsiString& name, bool bIgnoreExt)
{
	if (bIgnoreExt){
        if (start_folder){
            for (TElTreeItem* node=start_folder->GetFirstChild(); node; node=start_folder->GetNextChild(node)){
                AnsiString nm = ChangeFileExt(node->Text,"");
                if (nm==name) return node;
            }
        }else{
            for (TElTreeItem* node=tv->Items->GetFirstNode(); node; node=node->GetNextSibling()){
                AnsiString nm = ChangeFileExt(node->Text,"");
                if (nm==name) return node;
            }
        }
    }else{
        if (start_folder){
            for (TElTreeItem* node=start_folder->GetFirstChild(); node; node=start_folder->GetNextChild(node))
                if (node->Text==name) return node;
        }else{
            for (TElTreeItem* node=tv->Items->GetFirstNode(); node; node=node->GetNextSibling())
                if (node->Text==name) return node;
        }
    }
    return 0;
}
//---------------------------------------------------------------------------

TElTreeItem* FOLDER::FindItem(TElTree* tv, LPCSTR full_name, TElTreeItem** last_valid_node, int* last_valid_idx)
{
	int cnt = _GetItemCount(full_name,'\\');
    if (cnt<=0) return 0;

    // find folder item
    int itm = 0;
	char fld[64];
	TElTreeItem* node = 0;
    TElTreeItem* last_node = 0;
    do{
    	_GetItem(full_name,itm++,fld,'\\');
        last_node = node;
        node = FindItemInFolder(tv,node,fld);
    }while (node&&(itm<cnt));

    if(!node){
		if (last_valid_node) *last_valid_node=last_node;
        if (last_valid_idx) *last_valid_idx=--itm;
    }
    return node;
}
//---------------------------------------------------------------------------

TElTreeItem* FOLDER::FindFolder(TElTree* tv, LPCSTR full_name, TElTreeItem** last_valid_node, int* last_valid_idx)
{
	int cnt = _GetItemCount(full_name,'\\');
    if (cnt<=0) return 0;

    // find folder item
    int itm = 0;
	char fld[64];
	TElTreeItem* node = 0;
    TElTreeItem* last_node = 0;
    do{
    	_GetItem(full_name,itm++,fld,'\\');
        last_node = node;
        node = FindItemInFolder(TYPE_FOLDER,tv,node,fld);
    }while (node&&(itm<cnt));

    if(!node){
		if (last_valid_node) *last_valid_node=last_node;
        if (last_valid_idx) *last_valid_idx=--itm;
    }
    return node;
}
//---------------------------------------------------------------------------

TElTreeItem* FOLDER::FindObject(TElTree* tv, LPCSTR full_name, TElTreeItem** last_valid_node, int* last_valid_idx, bool bIgnoreExt)
{
	int cnt = _GetItemCount(full_name,'\\'); cnt--;
    if (cnt<0) return 0;

    // find folder item
    int itm = 0;
    char fld[64];
    TElTreeItem* node = 0;
    TElTreeItem* last_node = 0;
    if (cnt){
        do{
            _GetItem(full_name,itm++,fld,'\\');
            last_node = node;
            node = FindItemInFolder(TYPE_FOLDER,tv,node,fld);
        }while (node&&(itm<cnt));
    }

    if(cnt&&!node){
		if (last_valid_node) *last_valid_node=last_node;
        if (last_valid_idx) *last_valid_idx=--itm;
    }else{
    	// find object item if needed
        char obj[64];
        _GetItem(full_name,cnt,obj,'\\');
        last_node = node;
        node = FindItemInFolder(TYPE_OBJECT,tv,node,obj,bIgnoreExt);
        if (!node){
            if (last_valid_node) *last_valid_node=last_node;
            if (last_valid_idx) *last_valid_idx=itm;
        }
    }

    return node;
}
//---------------------------------------------------------------------------

TElTreeItem* FOLDER::AppendFolder(TElTree* tv, LPCSTR full_name)
{
    int idx=0;
	TElTreeItem* last_node=0;
    TElTreeItem* node = FindFolder(tv,full_name,&last_node,&idx);

    if (node) return node;

	char fld[64];
	int cnt = _GetItemCount(full_name,'\\');
	node = last_node;
    for (int itm=idx; itm<cnt; itm++){
    	_GetItem(full_name,itm,fld,'\\');
		node=tv->Items->AddChildObject(node,fld,(void*)TYPE_FOLDER);
    }
	return node;
}
//---------------------------------------------------------------------------

TElTreeItem* FOLDER::AppendObject(TElTree* tv, LPCSTR full_name)
{
    int idx=0;
	TElTreeItem* last_node=0;
	char fld[128];
	int fld_cnt = _GetItemCount(full_name,'\\')-1;
    _GetItems(full_name,0,fld_cnt,fld,'\\');
    TElTreeItem* fld_node = fld[0]?FindFolder(tv,fld,&last_node,&idx):0;

    if (!fld_node){
	    fld_node = last_node;
    	for (int itm=idx; itm<fld_cnt; itm++){
    		_GetItem(full_name,itm,fld,'\\');
			fld_node=tv->Items->AddChildObject(fld_node,fld,(void*)TYPE_FOLDER);
    	}
    }
	char obj[64];
	_GetItem(full_name,fld_cnt,obj,'\\');
    if (FindItemInFolder(TYPE_OBJECT,tv,fld_node,obj)) return 0;
	return tv->Items->AddChildObject(fld_node,obj,(void*)TYPE_OBJECT);
}
//---------------------------------------------------------------------------

void FOLDER::GenerateFolderName(TElTree* tv, TElTreeItem* node, AnsiString& name,LPCSTR pref)
{
	name = pref;
    int cnt = 0;
    while (FindItemInFolder(TYPE_FOLDER,tv,node,name))
    	name.sprintf("%s_%02d",pref,cnt++);
}
//---------------------------------------------------------------------------

void FOLDER::GenerateObjectName(TElTree* tv, TElTreeItem* node, AnsiString& name,LPCSTR pref)
{
	name = pref;
    int cnt = 0;
    while (FindItemInFolder(TYPE_OBJECT,tv,node,name))
    	name.sprintf("%s_%02d",pref,cnt++);
}
//---------------------------------------------------------------------------

LPCSTR FOLDER::ReplacePart(LPCSTR old_name, LPCSTR ren_part, int level, LPSTR dest)
{
    VERIFY(level<_GetItemCount(old_name,'\\'));
    _ReplaceItem(old_name,level,ren_part,dest,'\\');
    return dest;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Drag'n'Drop
//---------------------------------------------------------------------------
void FOLDER::DragDrop(TObject *Sender, TObject *Source, int X, int Y)
{
	TElTree* tv = dynamic_cast<TElTree*>(Sender); VERIFY(Sender);
	TElTreeItem* tgt_folder = tv->GetItemAt(X, Y, 0, 0);
    if (tgt_folder&&(IsObject(tgt_folder))) tgt_folder=tgt_folder->Parent;

    AnsiString base_name;
    MakeName(tgt_folder,0,base_name,true);
    AnsiString cur_fld_name=base_name;
    TElTreeItem* cur_folder=tgt_folder;

    int drg_level=DragItem->Level;

    TElTreeItem* item = DragItem;
    do{
    	DWORD type = DWORD(item->Data);
		TElTreeItem* pNode = FindItemInFolder(type,tv,cur_folder,item->Text);
		if (pNode&&IsObject(item)){
            item=item->GetNext();
        	continue;
        }

        if (!pNode) pNode = tv->Items->AddChildObject(cur_folder,item->Text,(TObject*)type);
		if (IsFolder(item)){
        	cur_folder = pNode;
		    MakeName(cur_folder,0,cur_fld_name,true);
            item=item->GetNext();
        }else{
        	// rename
		    AnsiString old_name, new_name;
		    MakeName(item,0,old_name,false);
		    MakeName(pNode,0,new_name,false);

            DragAction(old_name.c_str(),new_name.c_str());

            TElTreeItem* parent=item->Parent;
            // get next item && delete existance
            TElTreeItem* next=item->GetNext();
			item->Delete();

            if (parent&&((parent->GetLastChild()==item)||(0==parent->ChildrenCount))){
	            if (0==parent->ChildrenCount) parent->Delete();
	        	cur_folder = cur_folder?cur_folder->Parent:0;
            }

            item=next;
        }
    }while(item&&(item->Level>drg_level));
}
//---------------------------------------------------------------------------

void FOLDER::DragOver(TObject *Sender, TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
	TElTree* tv = dynamic_cast<TElTree*>(Sender); VERIFY(Sender);
	TElTreeItem* tgt;
    TElTreeItem* src=DragItem;
    TSTItemPart IP;
    int HCol;
    if (!DragItem) Accept = false;
  	else{
		tgt = tv->GetItemAt(X, Y, IP, HCol);
        if (tgt){
        	if (IsFolder(src)){
                bool b = true;
                for (TElTreeItem* itm=tgt->Parent; itm; itm=itm->Parent) if (itm==src){b=false; break;}
            	if (IsFolder(tgt)){
		        	Accept = b&&(tgt!=src)&&(src->Parent!=tgt);
                }else if (IsObject(tgt)){
		        	Accept = b&&(src!=tgt->Parent)&&(tgt!=src)&&(tgt->Parent!=src->Parent);
                }
            }else if (IsObject(src)){
            	if (IsFolder(tgt)){
		        	Accept = (tgt!=src)&&(src->Parent!=tgt);
                }else if (IsObject(tgt)){
		        	Accept = (tgt!=src)&&(src->Parent!=tgt->Parent);
                }
            }
        }else Accept = !!DragItem->Parent;
    }
}
//---------------------------------------------------------------------------

void FOLDER::StartDrag(TObject *Sender, TDragObject *&DragObject, TOnAfterDrag after_drag)
{
	DragAction = after_drag; R_ASSERT(DragAction);
	TElTree* tv = dynamic_cast<TElTree*>(Sender); VERIFY(Sender);
	if (tv->ItemFocused) 	DragItem = tv->ItemFocused;
  	else					DragItem = 0;
}
//---------------------------------------------------------------------------

