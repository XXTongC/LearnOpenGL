#pragma once

#include "engine/AssetRegistry.h"
#include "engine/EngineSubsystem.h"

namespace GLengine
{
	class AssetSubsystem : public EngineSubsystem
	{
	public:
		AssetSubsystem() = default;
		~AssetSubsystem() override = default;

		bool initialize(EngineContext& context) override;
		void tick(EngineContext& context) override;
		void shutdown(EngineContext& context) override;
		const char* getDebugName() const override;
		bool isInitializedForDiagnostics() const override;
		int getTickCountForDiagnostics() const override;

		AssetRegistry& getRegistry();
		const AssetRegistry& getRegistry() const;
		void clear();
		bool isInitialized() const;
		int getTickCount() const;

	private:
		AssetRegistry mRegistry{};
		int mTickCount{ 0 };
		bool mInitialized{ false };
	};
}
