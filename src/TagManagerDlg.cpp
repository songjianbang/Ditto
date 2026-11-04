// TagManagerDlg.cpp: add, rename and delete tag names.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CP_Main.h"
#include "Clip.h"
#include "Pinyin.h"
#include "TagEditDlg.h"
#include "TagManagerDlg.h"

#define IDC_TAGMGR_LIST   40011
#define IDC_TAGMGR_NAME   40012
#define IDC_TAGMGR_ADD    40013
#define IDC_TAGMGR_RENAME 40014
#define IDC_TAGMGR_DELETE 40015

BEGIN_MESSAGE_MAP(CTagManagerDlg, CDialog)
	ON_BN_CLICKED(IDC_TAGMGR_ADD, &CTagManagerDlg::OnAdd)
	ON_BN_CLICKED(IDC_TAGMGR_RENAME, &CTagManagerDlg::OnRename)
	ON_BN_CLICKED(IDC_TAGMGR_DELETE, &CTagManagerDlg::OnDelete)
	ON_NOTIFY(NM_DBLCLK, IDC_TAGMGR_LIST, &CTagManagerDlg::OnListDblClk)
END_MESSAGE_MAP()

CTagManagerDlg::CTagManagerDlg(CWnd* pParent)
	: CDialog()
{
	BuildTemplate();
	InitModalIndirect((LPCDLGTEMPLATE)m_template, pParent);
}

CTagManagerDlg::~CTagManagerDlg()
{
}

void CTagManagerDlg::BuildTemplate()
{
	memset(m_template, 0, sizeof(m_template));

	DLGTEMPLATE* pT = (DLGTEMPLATE*)m_template;
	pT->style = DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_SETFONT;
	pT->dwExtendedStyle = 0;
	pT->cdit = 0;
	pT->x = 0;
	pT->y = 0;
	pT->cx = 280;
	pT->cy = 240;

	BYTE* p = (BYTE*)(pT + 1);

	*(WORD*)p = 0;
	p += sizeof(WORD);

	*(WORD*)p = 0;
	p += sizeof(WORD);

	const WCHAR* title = L"Manage Tags";
	int len = (int)wcslen(title) + 1;
	memcpy(p, title, len * sizeof(WCHAR));
	p += len * sizeof(WCHAR);

	*(WORD*)p = 9;
	p += sizeof(WORD);

	const WCHAR* face = L"MS Shell Dlg";
	len = (int)wcslen(face) + 1;
	memcpy(p, face, len * sizeof(WCHAR));
}

BOOL CTagManagerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CenterWindow();

	m_font.CreateStockObject(DEFAULT_GUI_FONT);

	CRect rc;
	GetClientRect(&rc);

	const int pad = 10;
	const int rowH = 22;
	const int btnH = 25;
	const int btnW = 72;

	int listBottom = rc.bottom - pad - btnH - rowH - 16;

	m_list.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
		CRect(pad, pad, rc.right - pad, listBottom), this, IDC_TAGMGR_LIST);
	m_list.SetFont(&m_font);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, _T("Tag"), LVCFMT_LEFT, rc.Width() - pad * 2 - 30);

	int nameTop = listBottom + 10;

	m_staticName.Create(_T("Name:"), WS_CHILD | WS_VISIBLE | SS_LEFT,
		CRect(pad, nameTop + 4, pad + 42, nameTop + 18), this);
	m_staticName.SetFont(&m_font);

	m_editName.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		CRect(pad + 44, nameTop, rc.right - pad, nameTop + rowH), this, IDC_TAGMGR_NAME);
	m_editName.SetFont(&m_font);

	int by = rc.bottom - pad - btnH;

	m_btnAdd.Create(_T("Add"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		CRect(pad, by, pad + btnW, by + btnH), this, IDC_TAGMGR_ADD);
	m_btnAdd.SetFont(&m_font);

	m_btnRename.Create(_T("Rename"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		CRect(pad + btnW + 6, by, pad + btnW * 2 + 6, by + btnH), this, IDC_TAGMGR_RENAME);
	m_btnRename.SetFont(&m_font);

	m_btnDelete.Create(_T("Delete"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		CRect(pad + btnW * 2 + 12, by, pad + btnW * 3 + 12, by + btnH), this, IDC_TAGMGR_DELETE);
	m_btnDelete.SetFont(&m_font);

	m_btnClose.Create(_T("Close"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		CRect(rc.right - pad - btnW, by, rc.right - pad, by + btnH), this, IDCANCEL);
	m_btnClose.SetFont(&m_font);

	LoadTags();

	return FALSE;
}

void CTagManagerDlg::LoadTags()
{
	m_tags.clear();
	m_list.DeleteAllItems();

	try
	{
		CppSQLite3Query q = theApp.m_db.execQuery(_T("SELECT tagText FROM TagNames ORDER BY tagText ASC"));

		while (!q.eof())
		{
			CString tag = q.fieldValue(0);
			tag.Trim();

			if (!tag.IsEmpty())
			{
				m_tags.push_back(tag);
				AddToList(tag);
			}

			q.nextRow();
		}
	}
	catch (CppSQLite3Exception& e)
	{
		Log(StrF(_T("Failed to load tag names: %s"), e.errorMessage()));
	}
}

void CTagManagerDlg::AddToList(const CString& tag)
{
	int index = m_list.InsertItem(m_list.GetItemCount(), tag);
	if (index >= 0)
	{
		m_list.SetItemData(index, (DWORD_PTR)(m_tags.size() - 1));
	}
}

CString CTagManagerDlg::GetSelectedTag()
{
	int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
	if (sel < 0)
	{
		return _T("");
	}

	DWORD_PTR data = m_list.GetItemData(sel);
	if (data >= m_tags.size())
	{
		return _T("");
	}

	return m_tags[(size_t)data];
}

//Puts the tag name into TagNames, keeping the pinyin columns in sync.
bool CTagManagerDlg::SaveTagName(const CString& tag)
{
	try
	{
		CString pinyin = GetPinyinFull(tag, true);
		CString initials = GetPinyinInitials(tag, true);

		if (pinyin.IsEmpty())
		{
			pinyin = tag;
		}

		CString value = tag;
		value.Replace(_T("'"), _T("''"));
		pinyin.Replace(_T("'"), _T("''"));
		initials.Replace(_T("'"), _T("''"));

		theApp.m_db.execDMLEx(_T("INSERT OR IGNORE INTO TagNames (tagText, lastUsedDate, pinyin, pinyinInitials) VALUES ('%s', %lld, '%s', '%s');"),
			(LPCTSTR)value,
			CTime::GetCurrentTime().GetTime(),
			(LPCTSTR)pinyin,
			(LPCTSTR)initials);

		return true;
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)
}

//Rewrites every clip carrying oldTag. An empty newTag removes the tag.
bool CTagManagerDlg::ReplaceTagOnClips(const CString& oldTag, const CString& newTag)
{
	std::vector<std::pair<int, CString> > updates;

	try
	{
		CString escaped = oldTag;
		escaped.Replace(_T("'"), _T("''"));

		{
			CppSQLite3Query q = theApp.m_db.execQueryEx(
				_T("SELECT lID, tags FROM Main WHERE tags LIKE '%%%s%%'"),
				(LPCTSTR)escaped);

			while (!q.eof())
			{
				int id = q.getIntField(0);
				CString tags = q.getStringField(1);

				std::vector<CString> list;
				CTagEditDlg::SplitTags(tags, list);

				bool changed = false;

				for (size_t i = 0; i < list.size(); i++)
				{
					if (list[i].CompareNoCase(oldTag) == 0)
					{
						if (newTag.IsEmpty())
						{
							list.erase(list.begin() + i);
						}
						else
						{
							list[i] = newTag;
						}

						changed = true;
						break;
					}
				}

				if (changed)
				{
					updates.push_back(std::make_pair(id, CTagEditDlg::JoinTags(list)));
				}

				q.nextRow();
			}
		}

		for (size_t i = 0; i < updates.size(); i++)
		{
			CString value = updates[i].second;
			value.Replace(_T("'"), _T("''"));

			theApp.m_db.execDMLEx(_T("UPDATE Main SET tags = '%s' WHERE lID = %d"),
				(LPCTSTR)value,
				updates[i].first);
		}
	}
	CATCH_SQLITE_EXCEPTION_AND_RETURN(false)

	return true;
}

void CTagManagerDlg::OnAdd()
{
	CString name;
	m_editName.GetWindowText(name);
	name.Trim();

	if (name.IsEmpty())
	{
		return;
	}

	if (CTagEditDlg::ContainsTag(m_tags, name))
	{
		MessageBox(StrF(_T("Tag \"%s\" already exists."), (LPCTSTR)name), _T("Ditto"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	if (SaveTagName(name))
	{
		m_editName.SetWindowText(_T(""));
		LoadTags();
	}
}

void CTagManagerDlg::OnRename()
{
	CString oldTag = GetSelectedTag();
	if (oldTag.IsEmpty())
	{
		return;
	}

	CString newTag;
	m_editName.GetWindowText(newTag);
	newTag.Trim();

	if (newTag.IsEmpty() || newTag.CompareNoCase(oldTag) == 0)
	{
		return;
	}

	if (CTagEditDlg::ContainsTag(m_tags, newTag))
	{
		MessageBox(StrF(_T("Tag \"%s\" already exists."), (LPCTSTR)newTag), _T("Ditto"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	bool ok = false;

	try
	{
		theApp.m_db.execDML(_T("BEGIN IMMEDIATE"));

		ok = ReplaceTagOnClips(oldTag, newTag);

		if (ok)
		{
			CString escaped = oldTag;
			escaped.Replace(_T("'"), _T("''"));

			theApp.m_db.execDMLEx(_T("DELETE FROM TagNames WHERE tagText = '%s'"), (LPCTSTR)escaped);
			ok = SaveTagName(newTag);
		}

		if (ok)
		{
			theApp.m_db.execDML(_T("COMMIT"));
		}
		else
		{
			theApp.m_db.execDML(_T("ROLLBACK"));
		}
	}
	catch (CppSQLite3Exception& e)
	{
		Log(StrF(_T("Failed to rename tag: %s"), e.errorMessage()));

		try
		{
			theApp.m_db.execDML(_T("ROLLBACK"));
		}
		catch (CppSQLite3Exception&)
		{
		}

		ok = false;
	}

	if (ok)
	{
		m_editName.SetWindowText(_T(""));
		LoadTags();
	}
	else
	{
		MessageBox(_T("Failed to rename the tag."), _T("Ditto"), MB_OK | MB_ICONERROR);
	}
}

void CTagManagerDlg::OnDelete()
{
	CString tag = GetSelectedTag();
	if (tag.IsEmpty())
	{
		return;
	}

	CString message = StrF(_T("Delete tag \"%s\" from all clips?"), (LPCTSTR)tag);
	if (MessageBox(message, _T("Ditto"), MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	bool ok = false;

	try
	{
		theApp.m_db.execDML(_T("BEGIN IMMEDIATE"));

		ok = ReplaceTagOnClips(tag, _T(""));

		if (ok)
		{
			CString escaped = tag;
			escaped.Replace(_T("'"), _T("''"));

			theApp.m_db.execDMLEx(_T("DELETE FROM TagNames WHERE tagText = '%s'"), (LPCTSTR)escaped);
			theApp.m_db.execDML(_T("COMMIT"));
		}
		else
		{
			theApp.m_db.execDML(_T("ROLLBACK"));
		}
	}
	catch (CppSQLite3Exception& e)
	{
		Log(StrF(_T("Failed to delete tag: %s"), e.errorMessage()));

		try
		{
			theApp.m_db.execDML(_T("ROLLBACK"));
		}
		catch (CppSQLite3Exception&)
		{
		}

		ok = false;
	}

	if (ok)
	{
		LoadTags();
	}
	else
	{
		MessageBox(_T("Failed to delete the tag."), _T("Ditto"), MB_OK | MB_ICONERROR);
	}
}

void CTagManagerDlg::OnListDblClk(NMHDR* pNMHDR, LRESULT* pResult)
{
	CString tag = GetSelectedTag();
	if (!tag.IsEmpty())
	{
		m_editName.SetWindowText(tag);
		m_editName.SetSel(0, -1);
		m_editName.SetFocus();
	}

	*pResult = 0;
}
