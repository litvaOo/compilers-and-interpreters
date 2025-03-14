from typing import Any, Optional


class Environment:
    def __init__(self, parent: Optional["Environment"] = None) -> None:
        self.vars: dict[str, Any] = {}
        self.parent = parent
        self.funcs: dict[str, Any] = {}

    def __contains__(self, item: str) -> bool:
        try:
            self.vars[item]
        except KeyError:
            return self.parent is not None and item in self.parent
        return True

    def __getitem__(self, item: str) -> Any:
        if item in self.vars:
            return self.vars[item]
        elif self.parent is not None:
            return self.parent[item]
        raise KeyError

    def set_local(self, name: str, val: Any) -> None:
        self.vars[name] = val

    def get_func(self, name: str) -> Any:
        if name in self.funcs:
            return self.funcs[name]
        if self.parent is not None:
            return self.parent.get_func(name)
        raise KeyError

    def set_func(self, name: str, val: Any) -> Any:
        self.funcs[name] = val

    def __setitem__(self, item: str, value: Any):
        if self.parent is not None and item in self.parent:
            self.parent[item] = value
        else:
            self.vars[item] = value

    def new_env(self):
        return Environment(self)
