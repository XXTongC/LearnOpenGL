#pragma once

#include <memory>

#include "engine/EngineSubsystem.h"

namespace GLengine
{
	class AssetRegistry;

	class AssetSubsystem : public EngineSubsystem
	{
	public:
		AssetSubsystem();
		~AssetSubsystem() override;

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
		std::unique_ptr<AssetRegistry> mRegistry{};
		int mTickCount{ 0 };
		bool mInitialized{ false };
	};
}
