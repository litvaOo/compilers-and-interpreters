from typing import Any, Optional


class Environment:
    def __init__(self, parent: Optional["Environment"] = None) -> None:
        self.vars: dict[str, Any] = {}
        self.parent = parent

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

    def __setitem__(self, item: str, value: Any):
        # if item == "theChar":
        #     __import__("ipdb").set_trace()
        if self.parent is not None and item in self.parent:
            self.parent[item] = value
        else:
            self.vars[item] = value

    def new_env(self):
        return Environment(self)
