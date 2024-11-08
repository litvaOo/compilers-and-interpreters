const std = @import("std");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();
    defer _ = gpa.deinit();

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
}
