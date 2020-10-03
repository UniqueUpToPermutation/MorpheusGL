#include <engine/glslpreprocessor.hpp>

#include <iostream>
#include <algorithm>
#include <sstream>

namespace Morpheus {

	std::string GLSLPreprocessorConfig::stringify(const GLSLPreprocessorConfig* overrides) const {
		std::stringstream ss;
		bool bVersionOverriden = false;
		
		for (auto& it : mDefines) {
			ss << "#define " << it.first << " " << it.second << std::endl;
		}
		if (overrides) {
			for (auto& it : overrides->mDefines) {
				ss << "#undef " << it.first << std::endl;
				ss << "#define " << it.first << " " << it.second << std::endl;
			}
		}
		return ss.str();
	}

	GLSLPreprocessorConfig::GLSLPreprocessorConfig() : bOverrideVersion(false),
		mVersionString() {
	}

	void GLSLPreprocessor::load(const std::string& source,
		const std::string& path,
		const GLSLPreprocessorConfig* overrides,
		std::stringstream* streamOut,
		GLSLPreprocessorOutput* output, 
		std::set<std::string>* alreadyVisited,
		bool bOverrideVersion,
		const std::string& preprocessorStr) {

		if (alreadyVisited->find(source) != alreadyVisited->end()) {
			return;
		}

		alreadyVisited->emplace(source);

		std::string contents;
		if (!mSourceInterface->tryFind(source, &contents)) {
			std::cout << "Unabled to find: " << source << std::endl;
			throw new std::runtime_error(std::string("Unable to find: ") + source);
		}

		size_t version_loc = contents.find("#version");
		if (version_loc == std::string::npos) {
			std::cout << source << ": Warning: #version tolken not found!" << std::endl;
			version_loc = 0;
		}

		size_t line_number = std::count(contents.begin(), contents.begin() + version_loc, '\n') + 1;
		size_t body_begin = contents.find('\n', version_loc);

		if (body_begin == std::string::npos) {
			std::cout << source << ": Warning: shader file is empty!" << std::endl;
			return;
		}

		// Write preprocessed string
		std::stringstream& ss = *streamOut;
		if (alreadyVisited->size() == 1) {
			bool bVersionOverriden = false;
			if (overrides) {
				bVersionOverriden = overrides->bOverrideVersion;
				if (bVersionOverriden)
					ss << "#version " << overrides->mVersionString << std::endl;
			}

			if (bOverrideVersion && !bVersionOverriden) {
				ss << "#version " << mConfig.mVersionString << std::endl;
			}

			if (!bOverrideVersion) {
				ss << contents.substr(0, body_begin) << std::endl;
			}

			ss << preprocessorStr << std::endl;
		
		}

		ss << std::endl << "#line 1 " << alreadyVisited->size() - 1 << std::endl; // Reset line numbers
		ss << contents.substr(body_begin);
		output->mSources.emplace_back(source);

		// Find all includes
		size_t include_pos = contents.find("#pragma include");
		while (include_pos != std::string::npos) {
			size_t startIndx = contents.find('\"', include_pos) + 1;
			if (startIndx == std::string::npos) {
				std::cout << source << ": Warning: #pragma include detected without include file!" << std::endl;
				return;
			}
			size_t endIndex = contents.find('\"', startIndx);
			if (endIndex == std::string::npos) {
				std::cout << source << ": Warning: unmatched quote in #pragma include!" << std::endl;
				return;
			}
			std::string includeSource = contents.substr(startIndx, endIndex - startIndx);
			std::string nextPath = path;

			size_t separator_i = includeSource.rfind('/');
			if (separator_i != std::string::npos) {
				nextPath = path + '/' + includeSource.substr(0, separator_i);
			}
			includeSource = path + '/' + includeSource;

			load(includeSource, nextPath, overrides, streamOut,
				output, alreadyVisited, bOverrideVersion, preprocessorStr);

			include_pos = contents.find("#pragma include", include_pos + 1);
		}
	}

	void GLSLPreprocessor::load(const std::string& source, GLSLPreprocessorOutput* output, const GLSLPreprocessorConfig* overrides) {
		std::string preprocessorStr = mConfig.stringify(overrides);

		bool bOverrideVersion = mConfig.bOverrideVersion;
		if (overrides) {
			bOverrideVersion = bOverrideVersion || overrides->bOverrideVersion;
		}

		std::set<std::string> alreadyVisited;
		std::string path = ".";

		size_t separator_i = source.rfind('/');
		if (separator_i != std::string::npos) {
			path = source.substr(0, separator_i);
		}

		std::stringstream streamOut;

		load(source, path, overrides,
			&streamOut, output, &alreadyVisited, bOverrideVersion, preprocessorStr);

		output->mContent = streamOut.str();
	}
}