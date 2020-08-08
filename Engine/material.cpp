#include "material.hpp"

#include <fstream>
#include <iostream>
#include <string>

using namespace nlohmann;
using namespace std;

namespace Morpheus {
    template <>
    ref<Material> duplicateRef<Material>(const ref<Material>& a)
    {
        if (IS_POOLED_<NODE_ENUM(Material)>::RESULT) {
            Material* mat = new Material();
            mat->mShader = a->mShader;
            mat->mUniformAssigments = a->mUniformAssigments;
            mat->mSamplerAssignments = a->mSamplerAssignments;
            return ref<Material>(mat);
        }
        else {
            auto h_a = a.getPoolHandle();
            auto pool = h_a.getPool();

            auto mat = pool->alloc();
            mat->mShader = a->mShader;
            mat->mUniformAssigments = a->mUniformAssigments;
            mat->mSamplerAssignments = a->mSamplerAssignments;
            return ref<Material>(mat);
        }
    }

    template <>
    Node duplicateToNode<Material>(const ref<Material>& a) {
        auto dup = duplicateRef(a);
        auto node = graph()->addNode(dup);
        content()->addContentNode(node);
        return node;
    }

    template <>
    Node duplicate<Material>(const Node& a) {
        auto desc = graph()->desc(a);
        assert(desc->type == NodeType::MATERIAL);
        auto node = duplicateToNode(desc->owner.reinterpret<Material>());
        return node;
    }

    ref<void> ContentFactory<Material>::load(const std::string& source, Node& loadInto) {
        ifstream f(source);

        if (!f.is_open()) {
            cout << "Error: failed to open " << source << "!" << endl;
            return ref<void>(nullptr);
        }

        json j;
        f >> j;
        f.close();

        string shaderSrc;
        j["shader"].get_to(shaderSrc);

        // Load the shader through the content manager
        ref<Shader> shaderRef;
        auto shaderNode = content()->load<Shader>(shaderSrc, &shaderRef);
        if (!shaderNode.valid()) {
            cout << "Error: could not load dependency " << shaderSrc << "!" << endl;
            return ref<void>(nullptr);
        }

        Material* mat = new Material();
        mat->mShader = shaderRef;

        // Perform an override of shader parameters
        if (j.contains("uniform_override")) {
            readUniformDefaults(j["uniform_override"], shaderRef.get(),
                &mat->mUniformAssigments);
            // Overwrite necessary things
            mat->mUniformAssigments = mat->mUniformAssigments.overwrite(shaderRef->defaultUniformAssignments());
        }
        else
            // Carry over default assignments
            mat->mUniformAssigments = shaderRef->defaultUniformAssignments();

        // Perform an override of shader sampler assignments
        if (j.contains("sampler_override")) {
            loadSamplerDefaults(j["sampler_override"], shaderRef.get(), &mat->mSamplerAssignments,
                content(), loadInto);
        }
        else
            // Carry over default assingments
            mat->mSamplerAssignments = shaderRef->defaultSamplerAssignments();

        // Add this shader as a child of this material
        graph()->createEdge(loadInto, shaderNode);

        // Return the material
        return ref<void>(mat);
    }
    void ContentFactory<Material>::unload(ref<void>& ref) {
        delete ref.reinterpretGet<Material>();
    }
    void ContentFactory<Material>::dispose() {
        delete this;
    }
}