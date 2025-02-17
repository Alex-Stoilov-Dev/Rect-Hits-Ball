#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <math.h>

const int WIDTH = 800, HEIGHT = 600;

typedef struct Ball {
    int circ_X, circ_Y, circ_R;
    float velY;
    float gravity;
} Ball;

typedef struct Rect {
    float rect_X, rect_Y;
    int width, height;
} Rect;

void drawCircle(SDL_Renderer* renderer, Ball* ball) {
    // Loop from the highest point on the circle to the lowest (SDL Y coordinate starts from top left and goes down, so going int -Y means Y is increasing);
    for (int y = ball->circ_Y - ball->circ_R; y <= ball->circ_Y + ball->circ_R; y++) {
        // Loop goes from the most left to the most right point
        for (int x = ball->circ_X - ball->circ_R; x <= ball->circ_X + ball->circ_R; x++) {
            // dx and dy are the coordinates you loop over minus the center;
            int dx = x - ball->circ_X;
            int dy = y - ball->circ_Y;

            // Checks if the x^2 + y^2 is <= r^2; to satisfy the equation of a circle (x-x_center)^2 + (y-y_center)^2 <= r^2
            if (dx * dx + dy * dy <= ball->circ_R * ball->circ_R) {
                SDL_RenderPoint(renderer, x, y);
            }
        }
    }
}

void render(SDL_Renderer* renderer, Ball* ball, Rect* rect) {
    // Set the background color to white
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // Clears the screen
    SDL_RenderClear(renderer);

    // Set the draw color to red
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    // Draws a rectangle
    SDL_FRect rect1 = { rect->rect_X, rect->rect_Y, rect->width, rect->height }; // X coordinate, Y coordinate, WIDTH, HEIGHT;

    // Fills the rectangle, takes in the renderer you have created, along with the address of the rectangle it has to draw;
    SDL_RenderFillRect(renderer, &rect1);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    drawCircle(renderer, ball); // X coordinate, Y coordinate, circle Radius, Velocity on the Y plane;

    // Present the renderer (display the drawn content)
    SDL_RenderPresent(renderer);
}

int checkCollision(Ball* ball, Rect* rect) {
    // Find the closest point on the rectangle to the center of the circle
    float closestX = fmax(rect->rect_X, fmin(ball->circ_X, rect->rect_X + rect->width));
    float closestY = fmax(rect->rect_Y, fmin(ball->circ_Y, rect->rect_Y + rect->height));

    // Calculate the distance between the circle's center and this closest point
    float distX = ball->circ_X - closestX;
    float distY = ball->circ_Y - closestY;
    float distance = sqrt(distX * distX + distY * distY);

    // If the distance is less than or equal to the circle's radius, there is a collision
    return distance <= ball->circ_R;
}

void handleCollision(Ball* ball, Rect* rect) {
    // Check if there is a collision between the ball and the rectangle
    if (checkCollision(ball, rect)) {
        // Reverse the ball's vertical velocity to simulate a bounce
        ball->velY = -ball->velY;

        // Set a small gravity value to simulate the effect of gravity after the bounce
        ball->gravity = 0.1;

        // Adjust the ball's position to be just above the rectangle to prevent it from getting stuck
        ball->circ_Y = rect->rect_Y - ball->circ_R;
    }
}

int main(int argc, char* argv[]) {
    // Initialize SDL in SDL2 you can SDL_Init(EVERYTHING), for SDL3 you can only init certain parts.
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    // Create SDL window essentially boiler plate.
    SDL_Window* window = SDL_CreateWindow("Tutorial", WIDTH, HEIGHT, SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window) {
        printf("Couldn't create window: %s\n", SDL_GetError());
        return 1;
    }

    // Create SDL renderer essentially boiler plate.
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer || !window) {
        printf("Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create an SDL_event with the name event, events are things like user input, window events, system events;
    SDL_Event event;
    int running = 1;

    Ball ball = {400, 450, 15, 0, 0}; // X coordinate, Y coordinate, Radius, Initial vertical velocity, Initial gravity
    Rect rect = {400, 400, 250, 50};  // X coordinate, Y coordinate, Width, Height

    // Main game loop
    while (running) {
        // Loop that looks for events, such as user input.
        while (SDL_PollEvent(&event)) {
            // Checks if the event is of type quit, in which case it quits the loop.
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            }
            // Square movement and logic:
            // If the event is of type key_down (which would be the case if you press a key); enters a switch case for the different possible cases.
            if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.key) {
                    case SDLK_ESCAPE: running = 0; break;
                    case SDLK_W: rect.rect_Y -= 10; break;
                    case SDLK_S: rect.rect_Y += 10; break;
                    case SDLK_A: rect.rect_X -= 10; break;
                    case SDLK_D: rect.rect_X += 10; break;
                }
            }
        }

        // Boundary checks for the rectangle
        if (rect.rect_X < 0) rect.rect_X = 0;  // Left boundary
        if (rect.rect_X + rect.width > WIDTH) rect.rect_X = WIDTH - rect.width;  // Right boundary
        if (rect.rect_Y < 0) rect.rect_Y = 0;  // Top boundary
        if (rect.rect_Y + rect.height > HEIGHT) rect.rect_Y = HEIGHT - rect.height;  // Bottom boundary

        // Update ball position and velocity
        ball.velY += ball.gravity;
        ball.circ_Y += ball.velY;

        // Boundary checks for the ball
        if (ball.circ_X < 0) ball.circ_X = 0;  // Left boundary
        if (ball.circ_X + ball.circ_R > WIDTH) ball.circ_X = WIDTH - ball.circ_R;  // Right boundary
        if (ball.circ_Y < 0) ball.circ_Y = 0;  // Top boundary
        if (ball.circ_Y + ball.circ_R > HEIGHT) {
            ball.circ_Y = HEIGHT - ball.circ_R;  // Bottom boundary
            ball.velY = 0;
            ball.gravity = 0;
        }

        handleCollision(&ball, &rect);
        render(renderer, &ball, &rect); // Call the render function

        // Delays inputs by 16 milliseconds, used to control the FPS;
        SDL_Delay(16);
    }

    // Destroys the renderer and window so they don't take up memory, and quits SDL.
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}