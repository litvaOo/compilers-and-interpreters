const std = @import("std");
const tokens = @import("tokens.zig");
const lexer = @import("lexer.zig");

pub fn main() !void {
    var gpa = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    const filename = args[1];
    const file = try std.fs.cwd().openFile(filename, std.fs.File.OpenFlags{ .mode = .read_only, .lock = .none });
    defer file.close();

    const buffer_size = (try file.stat()).size;
    const contents = try file.reader().readAllAlloc(allocator, buffer_size);
    defer allocator.free(contents);

    const stdout = std.io.getStdOut();
    try stdout.writeAll(contents);

    var Lexer = try lexer.init_lexer(allocator, contents);
    try Lexer.tokenize();
    for (try Lexer.tokens.toOwnedSlice()) |token| {
        std.debug.print("{}\n", .{token});
    }
}
