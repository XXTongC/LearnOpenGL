#pragma once

namespace GL_RUNTIME
{
	struct RuntimeVerificationConfig;

	class RuntimePBRProfileLineVerification
	{
	public:
		static void reportAppliedProfile(const RuntimeVerificationConfig& verification);
	};
}
