const HashMap = @import("std").StringHashMap;
const ResultType = @import("interpreter.zig").ResultType;
const std = @import("std");

pub const State = struct {
    vars: HashMap(ResultType),
    parent: ?*State,
    allocator: std.mem.Allocator,

    pub fn get_item(self: State, item: []const u8) ?ResultType {
        const res = self.vars.get(item);
        if (res == null) {
            if (self.parent != null) {
                return self.parent.?.get_item(item);
            }
        }
        return res;
    }

    pub fn set_item(self: *State, item: []const u8, value: ResultType) !void {
        if (self.parent != null) {
            if (self.parent.?.get_item(item) != null) {
                return self.parent.?.set_item(item, value);
            }
        }
        try self.vars.put(item, value);
    }

    pub fn get_child_env(self: *State) State {
        return State{ .vars = HashMap(ResultType).init(self.allocator), .parent = self, .allocator = self.allocator };
    }
};

pub fn state_init(parent: ?*State, allocator: std.mem.Allocator) State {
    return State{ .vars = HashMap(ResultType).init(allocator), .parent = parent, .allocator = allocator };
}
