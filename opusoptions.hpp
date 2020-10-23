

/**
 * User settings
 */
class cOpusOptions : public firy::cOptions {

public:
	cOpusOptions();

	virtual firy::spOptionResponse warning(const std::string& pModule, const std::string& pMessage, const std::string& pMessageDetail = "");
	virtual void error(const std::string& pModule, const std::string& pMessage, const std::string& pMessageDetail = "");
};
