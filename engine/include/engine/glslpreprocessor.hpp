#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <set>

namespace Morpheus {
	struct GLSLPreprocessorConfig {
		std::unordered_map<std::string, std::string> mDefines;
		bool bOverrideVersion;
		std::string mVersionString;

		std::string stringify(const GLSLPreprocessorConfig* overrides) const;

		GLSLPreprocessorConfig();
	};

	class IGLSLSourceLoader {
	public:
		virtual bool tryFind(const std::string& source, const std::string& path, std::string* contents) = 0;
	};

	struct GLSLPreprocessorOutput {
		std::string mSource;
		std::string mContent;
	};
	
	class GLSLPreprocessor {
	private:
		GLSLPreprocessorConfig mConfig;
		IGLSLSourceLoader* mSourceInterface;

		void load(const std::string& source,
			const std::string& path,
			std::vector<GLSLPreprocessorOutput>* output, 
			std::set<std::string>* alreadyVisited,
			bool bOverrideVersion,
			const std::string& preprocessorStr);

	public:
		inline GLSLPreprocessor(IGLSLSourceLoader* loader) : mSourceInterface(loader) {
		}

		inline GLSLPreprocessorConfig* config() { return &mConfig; }

		void load(const std::string& source, std::vector<GLSLPreprocessorOutput>* output, 
			const GLSLPreprocessorConfig* overrides = nullptr);
	};
}