#pragma once

#include <vector>

//Modal window used to add, rename and delete tag names.
//Renaming or deleting also rewrites the tag list stored on every clip.
class CTagManagerDlg : public CDialog
{
public:
	CTagManagerDlg(CWnd* pParent = NULL);
	virtual ~CTagManagerDlg();

protected:
	virtual BOOL OnInitDialog();

	afx_msg void OnAdd();
	afx_msg void OnRename();
	afx_msg void OnDelete();
	afx_msg void OnListDblClk(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

private:
	void BuildTemplate();
	void LoadTags();
	void AddToList(const CString& tag);
	CString GetSelectedTag();
	//Rewrites every clip that carries oldTag, removing it when newTag is empty.
	bool ReplaceTagOnClips(const CString& oldTag, const CString& newTag);
	bool SaveTagName(const CString& tag);

	BYTE m_template[512];
	std::vector<CString> m_tags;

	CListCtrl m_list;
	CEdit m_editName;
	CButton m_btnAdd;
	CButton m_btnRename;
	CButton m_btnDelete;
	CButton m_btnClose;
	CStatic m_staticName;
	CFont m_font;
};
