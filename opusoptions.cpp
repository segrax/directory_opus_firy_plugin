#include "stdafx.h"

cOpusOptions::cOpusOptions() {

}

firy::spOptionResponse cOpusOptions::warning(firy::spImage pImage, const std::string& pMessage, const std::string& pMessageDetail) {
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

void cOpusOptions::error(firy::spImage pImage, const std::string& pMessage, const std::string& pMessageDetail) {
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

firy::spOptionResponse cOpusOptions::savechanges(firy::spImage pImage, const std::string& pMessage) {
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

