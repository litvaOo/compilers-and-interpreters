from typing import Any, Optional


class Environment:
    def __init__(self, parent: Optional["Environment"] = None) -> None:
        self.vars: dict[str, Any] = {}
        self.parent = parent

    def __getitem__(self, item: str) -> Any:
        if item in self.vars:
            return self.vars[item]
        elif self.parent is not None:
            return self.parent[item]
        raise KeyError

    def __setitem__(self, item: str, value: Any):
        self.vars[item] = value

    def new_env(self):
        return Environment(self)
