#include <engine/glslpreprocessor.hpp>

#include <iostream>
#include <algorithm>
#include <sstream>

namespace Morpheus {

	std::string GLSLPreprocessorConfig::stringify(const GLSLPreprocessorConfig* overrides) const {
		std::stringstream ss;
		bool bVersionOverriden = false;
		
		if (overrides) {
			bVersionOverriden = overrides->bOverrideVersion;
			ss << "#version " << overrides->mVersionString;
		}

		if (bOverrideVersion && !bVersionOverriden) {
			ss << "#version " << mVersionString;
		}

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
		std::vector<GLSLPreprocessorOutput>* output, 
		std::set<std::string>* alreadyVisited,
		bool bOverrideVersion,
		const std::string& preprocessorStr) {

		if (alreadyVisited->find(source) != alreadyVisited->end()) {
			return;
		}

		alreadyVisited->emplace(source);

		std::string contents;
		if (!mSourceInterface->tryFind(source, path, &contents)) {
			throw new std::runtime_error(std::string("Unabled to find: ") + source);
		}

		size_t version_loc = contents.find("#version");
		if (version_loc == std::string::npos) {
			std::cout << source << ": Warning: #version tolken not found!" << std::endl;
			version_loc = 0;
		}

		size_t line_number = std::count(contents.begin(), contents.begin() + version_loc, '\n') + 1;
		size_t body_begin = contents.find('\n', version_loc);

		if (body_begin != std::string::npos) {
			std::cout << source << ": Warning: shader file is empty!" << std::endl;
			return;
		}

		// Write preprocessed string
		std::stringstream ss;
		if (!bOverrideVersion) {
			ss << contents.substr(0, body_begin);
		}

		ss << preprocessorStr << std::endl;
		ss << "#line 1" << std::endl; // Reset line numbers

		ss << contents.substr(body_begin);
		GLSLPreprocessorOutput out;
		out.mContent = ss.str();
		out.mSource = source;
		output->emplace_back(out);

		// Find all includes
		size_t include_pos = contents.find("#pragma include");
		if (include_pos != std::string::npos) {
			size_t startIndx = contents.find('\"', include_pos);
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
				nextPath = includeSource.substr(0, separator_i + 1);
				includeSource = includeSource.substr(separator_i + 1);
			}

			load(includeSource, nextPath, output, alreadyVisited, bOverrideVersion, preprocessorStr);
		}
	}

	void GLSLPreprocessor::load(const std::string& source, std::vector<GLSLPreprocessorOutput>* output, const GLSLPreprocessorConfig* overrides) {
		std::string preprocessorStr = mConfig.stringify(overrides);

		bool bOverrideVersion = mConfig.bOverrideVersion;
		if (overrides) {
			bOverrideVersion = bOverrideVersion || overrides->bOverrideVersion;
		}

		std::set<std::string> alreadyVisited;
		std::string path = "./";
		load(source, path, output, &alreadyVisited, bOverrideVersion, preprocessorStr);
	}
}