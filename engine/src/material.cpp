#include <engine/material.hpp>

#include <fstream>
#include <iostream>
#include <string>

using namespace nlohmann;
using namespace std;

namespace Morpheus {
	Material* Material::toMaterial() {
		return this;
	}

	std::string ContentFactory<Material>::getContentTypeString() const {
		return MORPHEUS_STRINGIFY(Material);
	}

    INodeOwner* ContentFactory<Material>::load(const std::string& source, Node loadInto) {
        
        cout << "Loading material " << source << "..." << endl;

        ifstream f(source);

        if (!f.is_open()) {
            cout << "Error: failed to open " << source << "!" << endl;
            return nullptr;
        }

        json j;
        f >> j;
        f.close();

        string shaderSrc;
        j["shader"].get_to(shaderSrc);

		if (shaderSrc.length() == 0)
			throw std::runtime_error("Shader source is empty!");

		if (shaderSrc[0] != '/') {
			string prefix_include_path = "";

			auto extract_ptr = source.find_last_of("\\/");
			if (extract_ptr != string::npos)
				prefix_include_path = source.substr(0, extract_ptr + 1);

			shaderSrc = prefix_include_path + shaderSrc;
		}

        // Load the shader through the content manager
        auto shader = content()->load<Shader>(shaderSrc);
        if (!shader) {
            cout << "Error: could not load dependency " << shaderSrc << "!" << endl;
            return nullptr;
        }

        Material* mat = new Material();
        mat->mShader = shader;

        // Perform an override of shader parameters
        if (j.contains("uniforms")) {
            readUniformDefaults(j["uniforms"], shader,
                &mat->mUniformAssigments);
            // Overwrite necessary things
            mat->mUniformAssigments = mat->mUniformAssigments.overwrite(shader->defaultUniformAssignments());
        }
        else
            // Carry over default assignments
            mat->mUniformAssigments = shader->defaultUniformAssignments();

        // Perform an override of shader sampler assignments
        if (j.contains("samplers")) {
            loadSamplerDefaults(j["samplers"], shader, &mat->mSamplerAssignments,
                content(), loadInto, source);
        }
        else
            // Carry over default assingments
            mat->mSamplerAssignments = shader->defaultSamplerAssignments();

        // Add this shader as a child of this material
		loadInto.addChild(shader->node());

        // Return the material
        return mat;
    }

    void ContentFactory<Material>::unload(INodeOwner* ref) {
        delete ref;
    }
    
    ContentFactory<Material>::~ContentFactory() {
	}

	DefaultMaterialShaderView::DefaultMaterialShaderView(Shader* shader) {
		mEnvironmentDiffuseSH.find(shader, "environmentDiffuseSH");
		mEnvironmentSpecular.find(shader, "environmentSpecular");
		
		mAlbedo.find(shader, "albedo");
		mRoughness.find(shader, "roughness");
		mMetalness.find(shader, "metalness");
		mNormal.find(shader, "normal");
		mDisplacement.find(shader, "displacement");

		bSampleMetalness.find(shader, "bSampleMetalness");
		mMetalnessDefault.find(shader, "metalnessDefault");
	}
}