#include "engine/PersistentIdPolicy.h"

#include <cctype>

using namespace GLengine;

std::string_view GLengine::persistentIdSourceToken(PersistentIdSource source)
{
	switch (source)
	{
	case PersistentIdSource::PresetAssigned:
		return "preset";
	case PersistentIdSource::LegacyMirrorDerived:
		return "legacy-mirror";
	case PersistentIdSource::ImportedAssetDerived:
		return "imported-asset";
	case PersistentIdSource::EditorCreatedGenerated:
		return "editor-created";
	default:
		return "unknown";
	}
}

std::string GLengine::sanitizePersistentIdSegment(std::string_view value, std::string_view fallback)
{
	if (value.empty())
	{
		value = fallback;
	}

	std::string result{};
	result.reserve(value.size());
	for (const unsigned char ch : value)
	{
		if (std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.')
		{
			result.push_back(static_cast<char>(std::tolower(ch)));
		}
		else
		{
			result.push_back('-');
		}
	}

	if (result.empty())
	{
		result.assign(fallback);
	}
	return result;
}

std::string GLengine::makePersistentId(
	std::string_view objectKind,
	PersistentIdSource source,
	std::string_view scope
)
{
	return makePersistentId(objectKind, source, scope, {});
}

std::string GLengine::makePersistentId(
	std::string_view objectKind,
	PersistentIdSource source,
	std::string_view scope,
	std::initializer_list<std::string_view> pathSegments
)
{
	std::string id = sanitizePersistentIdSegment(objectKind, "object");
	id += ":";
	id += persistentIdSourceToken(source);
	id += ":";
	id += sanitizePersistentIdSegment(scope, "default");

	for (const std::string_view segment : pathSegments)
	{
		if (segment.empty())
		{
			continue;
		}
		id += ":";
		id += sanitizePersistentIdSegment(segment);
	}

	return id;
}

std::string GLengine::makeIndexedPersistentIdPathSegment(
	std::string_view type,
	std::string_view name,
	int occurrence
)
{
	return sanitizePersistentIdSegment(type, "object")
		+ "-"
		+ sanitizePersistentIdSegment(name, "unnamed")
		+ "-"
		+ std::to_string(occurrence);
}
