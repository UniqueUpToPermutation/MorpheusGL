#include "cooktorrance.hpp"

namespace Morpheus {
	BEGIN_UNIFORM_LINK(CookTorranceShader)
	SET_WORLD("model");
	SET_VIEW("view");
	SET_PROJECTION("projection");
	SET_WORLDINVTRANSPOSE("modelInverseTranspose");
	LINK_UNIFORM(mSpecularColor, "specularColor");
	LINK_UNIFORM(mF0, "F0");
	LINK_UNIFORM(mRoughness, "roughness");
	LINK_UNIFORM(mK, "k");
	LINK_UNIFORM(mLightColor, "lightColor");
	LINK_UNIFORM(mAmbientStrength, "ambientStrength");
	LINK_UNIFORM(mLightIntensity, "lightIntensity");
	LINK_UNIFORM(mEyePosition, "eyePosition");
	LINK_UNIFORM(mLightPosition, "lightPosition");
	END_UNIFORM_LINK
}