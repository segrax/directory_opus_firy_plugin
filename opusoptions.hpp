

/**
 * User settings
 */
class cOpusOptions : public firy::cOptions {

public:
	cOpusOptions();
	virtual firy::spOptions clone() { return std::make_shared<cOpusOptions>(*std::dynamic_pointer_cast<cOpusOptions>(shared_from_this())); }

	virtual firy::spOptionResponse warning(firy::pImage pImage, const std::string& pMessage, const std::string& pMessageDetail = "");
	virtual void error(firy::pImage pImage, const std::string& pMessage, const std::string& pMessageDetail = "");

	virtual firy::spOptionResponse savechanges(firy::pImage pImage, const std::string& pMessage);
	virtual	firy::spOptionResponse savechangesExit(firy::pImage pImage, const std::string& pMessage);
};
