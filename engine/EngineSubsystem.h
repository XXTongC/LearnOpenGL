#pragma once

namespace GLengine
{
	struct EngineContext;

	class EngineSubsystem
	{
	public:
		virtual ~EngineSubsystem() = default;

		virtual bool initialize(EngineContext& context) = 0;
		virtual void tick(EngineContext& context) {}
		virtual void shutdown(EngineContext& context) = 0;

		virtual const char* getDebugName() const { return "EngineSubsystem"; }
		virtual bool isInitializedForDiagnostics() const { return false; }
		virtual int getTickCountForDiagnostics() const { return 0; }
	};
}
