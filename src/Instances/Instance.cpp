#include <RbxLib/Instances/Instance.h>

#include <string_view>

namespace RbxLib::Roblox {

Core::Result<Instance> Instance::FindFirstChild(const std::string_view name) const {
    const auto children = GetChildren();
    if (!children.Succeeded()) {
        return Core::Result<Instance>::Failure(children.Message());
    }
    for (const auto& child : children.Value()) {
        const auto childName = child.Name();
        if (!childName.Succeeded()) {
            return Core::Result<Instance>::Failure(childName.Message());
        }
        if (childName.Value() == name) {
            return Core::Result<Instance>::Success(child);
        }
    }
    return Core::Result<Instance>::Success(Instance{});
}

Core::Result<Instance> Instance::FindFirstChildFromPath(
    const std::string_view path) const {
    if (path.empty()) {
        return Core::Result<Instance>::Success(Instance{});
    }

    Instance current = *this;
    std::size_t start = 0;
    while (start < path.size()) {
        const auto separator = path.find('.', start);
        const auto end = separator == std::string_view::npos ? path.size() : separator;
        if (end == start) {
            return Core::Result<Instance>::Success(Instance{});
        }
        const auto child = current.FindFirstChild(path.substr(start, end - start));
        if (!child.Succeeded() || !child.Value().IsValid()) {
            return child;
        }
        current = child.Value();
        if (separator == std::string_view::npos) {
            break;
        }
        start = separator + 1;
    }
    return Core::Result<Instance>::Success(std::move(current));
}

Core::Result<Instance> Instance::FindFirstDescendant(
    const std::string_view name) const {
    const auto descendants = GetDescendants();
    if (!descendants.Succeeded()) {
        return Core::Result<Instance>::Failure(descendants.Message());
    }
    for (const auto& descendant : descendants.Value()) {
        const auto descendantName = descendant.Name();
        if (!descendantName.Succeeded()) {
            return Core::Result<Instance>::Failure(descendantName.Message());
        }
        if (descendantName.Value() == name) {
            return Core::Result<Instance>::Success(descendant);
        }
    }
    return Core::Result<Instance>::Success(Instance{});
}

Core::Result<Instance> Instance::FindFirstChildOfClass(
    const std::string_view className) const {
    const auto children = GetChildren();
    if (!children.Succeeded()) {
        return Core::Result<Instance>::Failure(children.Message());
    }
    for (const auto& child : children.Value()) {
        const auto childClass = child.ClassName();
        if (!childClass.Succeeded()) {
            return Core::Result<Instance>::Failure(childClass.Message());
        }
        if (childClass.Value() == className) {
            return Core::Result<Instance>::Success(child);
        }
    }
    return Core::Result<Instance>::Success(Instance{});
}

Core::Result<Instance> Instance::FindFirstChildWhichIsA(
    const std::string_view className) const {
    const auto children = GetChildren();
    if (!children.Succeeded()) {
        return Core::Result<Instance>::Failure(children.Message());
    }
    for (const auto& child : children.Value()) {
        const auto isA = child.IsA(className);
        if (!isA.Succeeded()) {
            return Core::Result<Instance>::Failure(isA.Message());
        }
        if (isA.Value()) {
            return Core::Result<Instance>::Success(child);
        }
    }
    return Core::Result<Instance>::Success(Instance{});
}

}
