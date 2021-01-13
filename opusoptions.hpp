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
