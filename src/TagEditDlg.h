#pragma once

#include <vector>

// Modal window used to edit the tags of a single clip.
// A tag can be typed in by hand or picked from the tags already in use.
class CTagEditDlg : public CDialog
{
public:
	CTagEditDlg(const CString& currentTags, const std::vector<CString>& knownTags, CWnd* pParent = NULL);
	virtual ~CTagEditDlg();

	// Comma separated tag list as edited by the user.
	CString GetTags() const { return m_resultTags; }

	// Splits a comma separated tag list, trimming and de-duplicating entries.
	static void SplitTags(const CString& tags, std::vector<CString>& out);
	static bool ContainsTag(const std::vector<CString>& list, const CString& tag);
	// Joins tags back into the comma separated form stored in the db.
	static CString JoinTags(const std::vector<CString>& tags);

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	afx_msg void OnAdd();
	afx_msg void OnRemove();

	DECLARE_MESSAGE_MAP()

private:
	void BuildTemplate();
	void RefreshLists();

	BYTE m_template[512];
	std::vector<CString> m_current;
	std::vector<CString> m_known;
	CString m_resultTags;

	CEdit m_editNew;
	CListBox m_lstCurrent;
	CListBox m_lstKnown;
	CButton m_btnAdd;
	CButton m_btnRemove;
	CButton m_btnOK;
	CButton m_btnCancel;
	CStatic m_staticNew;
	CStatic m_staticCurrent;
	CStatic m_staticKnown;
	CFont m_font;
};
