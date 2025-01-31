leftEdge = -420
rightEdge = 300
topEdge = 300
bottomEdge = -300
xStep = 7
yStep = 15

maxIter = 200

for y0 in range(topEdge, bottomEdge, -yStep):
    for x0 in range(leftEdge, rightEdge, xStep):
        y = 0
        x = 0
        i = 0
        char = " "
        while i < maxIter:
            x_x = (x * x) / 200
            y_y = (y * y) / 200
            if x_x + y_y > 800:
                char = str(i)
                if i > 9:
                    char = "@"
                i = maxIter
            y = x * y / 100 + y0
            x = x_x - y_y + x0
            i += 1
            # print(
            #     int(y) if y == int(y) else y,
            #     int(x) if int(x) == x else x,
            #     i,
            #     char,
            #     int(x_x) if x_x == int(x_x) else x_x,
            #     int(y_y) if y_y == int(y_y) else y_y,
            # )

        print(char, end="")
    print(y0)
    print()
