// TagEditDlg.cpp: editor for the tag list of a single clip.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TagEditDlg.h"

#define IDC_TAG_NEW     40001
#define IDC_TAG_CURRENT 40002
#define IDC_TAG_KNOWN   40003
#define IDC_TAG_ADD     40004
#define IDC_TAG_REMOVE  40005

BEGIN_MESSAGE_MAP(CTagEditDlg, CDialog)
	ON_BN_CLICKED(IDC_TAG_ADD, &CTagEditDlg::OnAdd)
	ON_BN_CLICKED(IDC_TAG_REMOVE, &CTagEditDlg::OnRemove)
	ON_LBN_DBLCLK(IDC_TAG_KNOWN, &CTagEditDlg::OnAdd)
	ON_LBN_DBLCLK(IDC_TAG_CURRENT, &CTagEditDlg::OnRemove)
END_MESSAGE_MAP()

CTagEditDlg::CTagEditDlg(const CString& currentTags, const std::vector<CString>& knownTags, CWnd* pParent)
	: CDialog()
{
	SplitTags(currentTags, m_current);
	m_known = knownTags;

	BuildTemplate();
	InitModalIndirect((LPCDLGTEMPLATE)m_template, pParent);
}

CTagEditDlg::~CTagEditDlg()
{
}

// Builds a minimal in memory dialog template so no .rc resource is required.
void CTagEditDlg::BuildTemplate()
{
	memset(m_template, 0, sizeof(m_template));

	DLGTEMPLATE* pT = (DLGTEMPLATE*)m_template;
	pT->style = DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_SETFONT;
	pT->dwExtendedStyle = 0;
	pT->cdit = 0;
	pT->x = 0;
	pT->y = 0;
	pT->cx = 300;
	pT->cy = 230;

	BYTE* p = (BYTE*)(pT + 1);

	// no menu
	*(WORD*)p = 0;
	p += sizeof(WORD);

	// default dialog class
	*(WORD*)p = 0;
	p += sizeof(WORD);

	const WCHAR* title = L"Tags";
	int len = (int)wcslen(title) + 1;
	memcpy(p, title, len * sizeof(WCHAR));
	p += len * sizeof(WCHAR);

	// DS_SETFONT payload
	*(WORD*)p = 9;
	p += sizeof(WORD);

	const WCHAR* face = L"MS Shell Dlg";
	len = (int)wcslen(face) + 1;
	memcpy(p, face, len * sizeof(WCHAR));
}

BOOL CTagEditDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(_T("Tags"));
	CenterWindow();

	m_font.CreateStockObject(DEFAULT_GUI_FONT);

	CRect rc;
	GetClientRect(&rc);

	const int pad = 10;
	const int rowH = 22;
	const int labelH = 15;
	const int btnH = 25;
	const int btnW = 72;

	// new tag row
	m_staticNew.Create(_T("New tag:"), WS_CHILD | WS_VISIBLE | SS_LEFT,
		CRect(pad, pad + 4, pad + 62, pad + 4 + labelH), this);
	m_staticNew.SetFont(&m_font);

	m_editNew.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
		CRect(pad + 64, pad, rc.right - pad - btnW - 8, pad + rowH), this, IDC_TAG_NEW);
	m_editNew.SetFont(&m_font);

	m_btnAdd.Create(_T("Add"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		CRect(rc.right - pad - btnW, pad, rc.right - pad, pad + rowH), this, IDC_TAG_ADD);
	m_btnAdd.SetFont(&m_font);

	// two lists side by side
	int listTop = pad + rowH + 10;
	int listBottom = rc.bottom - pad - btnH - 10;
	int listW = (rc.right - pad * 3) / 2;

	m_staticCurrent.Create(_T("Tags on this clip:"), WS_CHILD | WS_VISIBLE | SS_LEFT,
		CRect(pad, listTop, pad + listW, listTop + labelH), this);
	m_staticCurrent.SetFont(&m_font);

	m_lstCurrent.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
		CRect(pad, listTop + labelH + 3, pad + listW, listBottom), this, IDC_TAG_CURRENT);
	m_lstCurrent.SetFont(&m_font);

	m_staticKnown.Create(_T("Existing tags:"), WS_CHILD | WS_VISIBLE | SS_LEFT,
		CRect(pad * 2 + listW, listTop, rc.right - pad, listTop + labelH), this);
	m_staticKnown.SetFont(&m_font);

	m_lstKnown.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
		CRect(pad * 2 + listW, listTop + labelH + 3, rc.right - pad, listBottom), this, IDC_TAG_KNOWN);
	m_lstKnown.SetFont(&m_font);

	// bottom buttons
	int by = rc.bottom - pad - btnH;

	m_btnRemove.Create(_T("Remove"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		CRect(pad, by, pad + btnW, by + btnH), this, IDC_TAG_REMOVE);
	m_btnRemove.SetFont(&m_font);

	m_btnCancel.Create(_T("Cancel"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		CRect(rc.right - pad - btnW, by, rc.right - pad, by + btnH), this, IDCANCEL);
	m_btnCancel.SetFont(&m_font);

	m_btnOK.Create(_T("OK"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		CRect(rc.right - pad - btnW * 2 - 8, by, rc.right - pad - btnW - 8, by + btnH), this, IDOK);
	m_btnOK.SetFont(&m_font);

	RefreshLists();

	m_editNew.SetFocus();

	return FALSE;
}

bool CTagEditDlg::ContainsTag(const std::vector<CString>& list, const CString& tag)
{
	for (size_t i = 0; i < list.size(); i++)
	{
		if (list[i].CompareNoCase(tag) == 0)
		{
			return true;
		}
	}

	return false;
}

void CTagEditDlg::SplitTags(const CString& tags, std::vector<CString>& out)
{
	out.clear();

	CString work = tags;
	work.Trim();

	int start = 0;
	while (start < work.GetLength())
	{
		int idx = work.Find(_T(','), start);

		CString part;
		if (idx < 0)
		{
			part = work.Mid(start);
			start = work.GetLength();
		}
		else
		{
			part = work.Mid(start, idx - start);
			start = idx + 1;
		}

		part.Trim();
		if (!part.IsEmpty() && !ContainsTag(out, part))
		{
			out.push_back(part);
		}
	}
}

CString CTagEditDlg::JoinTags(const std::vector<CString>& tags)
{
	CString out;

	for (size_t i = 0; i < tags.size(); i++)
	{
		if (!out.IsEmpty())
		{
			out += _T(",");
		}

		out += tags[i];
	}

	return out;
}

void CTagEditDlg::RefreshLists()
{
	m_lstCurrent.ResetContent();
	for (size_t i = 0; i < m_current.size(); i++)
	{
		m_lstCurrent.AddString(m_current[i]);
	}

	m_lstKnown.ResetContent();
	for (size_t i = 0; i < m_known.size(); i++)
	{
		if (!ContainsTag(m_current, m_known[i]))
		{
			m_lstKnown.AddString(m_known[i]);
		}
	}
}

void CTagEditDlg::OnAdd()
{
	CString text;
	m_editNew.GetWindowText(text);
	text.Trim();

	if (!text.IsEmpty())
	{
		std::vector<CString> entered;
		SplitTags(text, entered);

		for (size_t i = 0; i < entered.size(); i++)
		{
			if (!ContainsTag(m_current, entered[i]))
			{
				m_current.push_back(entered[i]);
			}
		}

		m_editNew.SetWindowText(_T(""));
	}

	int sel = m_lstKnown.GetCurSel();
	if (sel != LB_ERR)
	{
		CString known;
		m_lstKnown.GetText(sel, known);

		if (!known.IsEmpty() && !ContainsTag(m_current, known))
		{
			m_current.push_back(known);
		}
	}

	RefreshLists();
	m_editNew.SetFocus();
}

void CTagEditDlg::OnRemove()
{
	int sel = m_lstCurrent.GetCurSel();
	if (sel == LB_ERR)
	{
		return;
	}

	if (sel >= 0 && sel < (int)m_current.size())
	{
		m_current.erase(m_current.begin() + sel);
	}

	RefreshLists();
}

void CTagEditDlg::OnOK()
{
	// pick up text that was typed but never added with the button
	CString pending;
	m_editNew.GetWindowText(pending);
	pending.Trim();

	if (!pending.IsEmpty())
	{
		std::vector<CString> entered;
		SplitTags(pending, entered);

		for (size_t i = 0; i < entered.size(); i++)
		{
			if (!ContainsTag(m_current, entered[i]))
			{
				m_current.push_back(entered[i]);
			}
		}
	}

	m_resultTags = JoinTags(m_current);

	CDialog::OnOK();
}
