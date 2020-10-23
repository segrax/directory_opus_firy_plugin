

/**
 * User settings
 */
class cOpusOptions : public firy::cOptions {

public:
	cOpusOptions();
	virtual firy::spOptions clone() { return std::make_shared<cOpusOptions>(*std::dynamic_pointer_cast<cOpusOptions>(shared_from_this())); }

	virtual firy::spOptionResponse warning(const std::string& pModule, const std::string& pMessage, const std::string& pMessageDetail = "");
	virtual void error(const std::string& pModule, const std::string& pMessage, const std::string& pMessageDetail = "");
};
