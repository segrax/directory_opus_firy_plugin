/*
	Copyright (c) 2020-2021 Robert Crossfield

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "stdafx.h"

cOpusOptions::cOpusOptions() {

	mAutoSaveSource = true;
	mAutoSaveSourceExit = true;
}

firy::spOptionResponse cOpusOptions::warning(firy::pImage pImage, const std::string& pMessage, const std::string& pMessageDetail) {
	if (!mWarningShow)
		return std::make_shared<firy::cOptionResponse>(true);

	auto msg = s2ws(pMessage);
	msg += s2ws(pMessageDetail);

	auto mod = L"Error, continue with " + s2ws(pImage->sourceID());

	SHOWREQUESTDLGDATA data;
	memset(&data, 0, sizeof(data));
	data.cbSize = sizeof(data);
	data.hwndParent = DOpus.GetFunctionWindow(0);
	data.pszTitle = mod.c_str();
	data.pszMessage = msg.c_str();
	data.pszButtons = BUTTONS_OKCANCEL;
	data.hIcon = HICON_WARNING;

	if (!DOpusUtil.ShowRequestDlg(&data)) {
		return std::make_shared<firy::cOptionResponse>( true );
	}

	return std::make_shared<firy::cOptionResponse>(false);

}

void cOpusOptions::error(firy::pImage pImage, const std::string& pMessage, const std::string& pMessageDetail) {
	if (!mErrorShow)
		return;

	auto msg = s2ws(pMessage);
	msg += s2ws(pMessageDetail);

	auto mod = L"Error in " + s2ws(pImage->sourceID());

	SHOWREQUESTDLGDATA data;
	memset(&data, 0, sizeof(data));
	data.cbSize = sizeof(data);
	data.hwndParent = DOpus.GetFunctionWindow(0);
	data.pszTitle = mod.c_str();
	data.pszMessage = msg.c_str();
	data.pszButtons = BUTTONS_OK;
	data.hIcon = HICON_ERROR;

	DOpusUtil.ShowRequestDlg(&data);
}

firy::spOptionResponse cOpusOptions::savechanges(firy::pImage pImage, const std::string& pMessage) {
	if (mAutoSaveSource)
		return std::make_shared<firy::cOptionResponse>(false);

	auto mod = L"Save changes to " + s2ws(pImage->sourceID());

	SHOWREQUESTDLGDATA data;
	memset(&data, 0, sizeof(data));
	data.cbSize = sizeof(data);
	data.hwndParent = DOpus.GetFunctionWindow(0);
	data.pszTitle = L"Save Changes";
	data.pszMessage = mod.c_str();
	data.pszButtons = BUTTONS_OKCANCEL;
	data.hIcon = HICON_WARNING;

	if (!DOpusUtil.ShowRequestDlg(&data)) {
		return std::make_shared<firy::cOptionResponse>(true);
	}

	return std::make_shared<firy::cOptionResponse>(false);
}

firy::spOptionResponse cOpusOptions::savechangesExit(firy::pImage pImage, const std::string& pMessage) {
	if (mAutoSaveSourceExit)
		return std::make_shared<firy::cOptionResponse>(false);

	auto mod = L"Save changes to " + s2ws(pImage->sourceID());

	SHOWREQUESTDLGDATA data;
	memset(&data, 0, sizeof(data));
	data.cbSize = sizeof(data);
	data.hwndParent = DOpus.GetFunctionWindow(0);
	data.pszTitle = L"Save Changes";
	data.pszMessage = mod.c_str();
	data.pszButtons = BUTTONS_OKCANCEL;
	data.hIcon = HICON_WARNING;

	if (!DOpusUtil.ShowRequestDlg(&data)) {
		return std::make_shared<firy::cOptionResponse>(true);
	}

	return std::make_shared<firy::cOptionResponse>(false);
}
