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

template<class tType, 
		 class tContainer = std::vector<tType>> 
	
	class cSafeContainer {
		using tCallback = std::function<bool(tType)>;
		using tCallbackNoRet = std::function<void(tType)>;
	public:

		cSafeContainer() {

		}

		cSafeContainer(cSafeContainer& pObj) {
			tLockGuard lock(pObj.mLock);

			mContainer = pObj.mContainer;
		}

		tContainer copy() {
			tLockGuard lock(mLock);

			tContainer result = mContainer;
			return result;
		}

		/**
		 * Push to container
		 */
		void push(tType pData) {
			tLockGuard lock(mLock);

			if constexpr (std::is_same_v<tContainer, std::vector<tType>>) {
				mContainer.push_back(pData);
			} else if constexpr (std::is_same_v<tContainer, std::queue<tType>>) {
				mContainer.push(pData);
			}
			else {
				mContainer.insert(pData);
			}
		}

		/**
		 * Take front item
		 */
		tType pop() {
			tLockGuard lock(mLock);

			if (!mContainer.size())
				return 0;

			auto ret = mContainer.front();
			if constexpr (std::is_same_v<tContainer, std::vector<tType>>) {
				mContainer.erase(mContainer.begin());
			} else if constexpr (std::is_same_v<tContainer, std::queue<tType>>) {
				mContainer.pop();
			}
			return ret;
		}

		/**
		 * Put a lock in place and return a reference to the container
		 */
		tContainer& lockContainer() {
			mLock.lock();
			return mContainer;
		}

		/**
		 * Remove the lock
		 */
		void unlock() {
			mLock.unlock();
		}

		/**
		 * Clear all entries
		 */
		void clear() {
			tLockGuard lock(mLock);
			mContainer.clear();
		}

		/**
		 * Return size of container
		 */
		size_t size() {
			tLockGuard lock(mLock);
			return mContainer.size();
		}

		/**
		 * Iterate each item via the callback until we find one
		 */
		tType find_if(tCallback pCallback) {
			tLockGuard lock(mLock);

			auto iter = std::find_if(mContainer.begin(), mContainer.end(), pCallback);
			if (iter == mContainer.end())
				return {};

			return *iter;
		}

		/**
		 * Remove an entry if the callback returns true
		 */
		void remove_if(tCallback pCallback) {
			tLockGuard lock(mLock);

			auto remove = std::remove_if(mContainer.begin(), mContainer.end(), pCallback);
			mContainer.erase(remove, mContainer.end());
		}

		/**
		 * Call the callback for each member of the container
		 */
		void iterate(tCallbackNoRet pCallback) {
			tLockGuard lock(mLock);

			for (auto& item : mContainer) {
				pCallback(item);
			}
		}

	private:
		tContainer mContainer;
		std::mutex mLock;
};
