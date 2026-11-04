#pragma once

//Returns true when the string holds at least one hanzi whose pinyin is known.
bool HasKnownPinyin(const CString& text);

//Builds a "gongzuo" style string out of hanzi. Ascii letters and digits are kept, lowercased.
CString GetPinyinFull(const CString& text, bool bKeepOther = true);

//Builds the first letter of every syllable, e.g. "gz" for gong zuo.
CString GetPinyinInitials(const CString& text, bool bKeepOther = true);
