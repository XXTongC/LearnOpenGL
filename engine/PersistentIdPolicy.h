#pragma once

#include <initializer_list>
#include <string>
#include <string_view>

namespace GLengine
{
	enum class PersistentIdSource
	{
		PresetAssigned,
		LegacyMirrorDerived,
		ImportedAssetDerived,
		EditorCreatedGenerated
	};

	std::string_view persistentIdSourceToken(PersistentIdSource source);
	std::string sanitizePersistentIdSegment(std::string_view value, std::string_view fallback = "unnamed");
	std::string makePersistentId(
		std::string_view objectKind,
		PersistentIdSource source,
		std::string_view scope
	);
	std::string makePersistentId(
		std::string_view objectKind,
		PersistentIdSource source,
		std::string_view scope,
		std::initializer_list<std::string_view> pathSegments
	);
	std::string makeIndexedPersistentIdPathSegment(
		std::string_view type,
		std::string_view name,
		int occurrence
	);
}
