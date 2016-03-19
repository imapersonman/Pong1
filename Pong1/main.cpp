//
//  main.cpp
//  Pong1
//
//  Created by Koissi Adjorlolo on 2/20/16.
//  Copyright © 2016 centuryapps. All rights reserved.
//

#include <iostream>
#include <SDL2/SDL.h>

typedef enum
{
    PaddlePosition_Left,
    PaddlePosition_Right
} PaddlePosition;

typedef struct
{
    int x, y;
} Vector2i;

typedef struct
{
    SDL_Rect rect;
    Vector2i screenPosition;
    PaddlePosition paddlePosition;
    int score;
} Paddle;

typedef struct
{
    SDL_Rect rect;
    Vector2i screenPosition;
    Vector2i velocity;
} Ball;

static const int WINDOW_WIDTH = 1024;
static const int WINDOW_HEIGHT = 768;
static const double MS_PER_UPDATE = 1000 / 60;
static const int PADDLE_WIDTH = 20;
static const int PADDLE_HEIGHT = 200;
static const int BALL_RADIUS = 10;
static const int PADDLE_SPEED = 5;

static Paddle createPaddle(PaddlePosition pPosition, int score);
static Ball createBall();
static void update();
static void updatePaddle(Paddle &paddle);
static void updateBall(Ball &ball);
static void updateControls();
static void updateState();
static void keepBallInBounds(Ball &ball);
static void collideBallWithPaddle(Ball &ball, Paddle &paddle);
static void render(SDL_Renderer *renderer);
static void renderPaddle(SDL_Renderer *renderer, Paddle paddle);
static void renderBall(SDL_Renderer *renderer, Ball ball);
static void reset();
static int getIntSign(int num);
static Vector2i getBallPosAtBounce(Ball &ball);
static void movePaddleTo(Paddle &paddle, Vector2i position);
static void automatePaddle(Paddle &paddle);
static void controlPaddle(Paddle &paddle);

static SDL_Window *gWindow = NULL;
static SDL_Renderer *gRenderer = NULL;

Paddle paddle1 = createPaddle(PaddlePosition_Left, 0);
Paddle paddle2 = createPaddle(PaddlePosition_Right, 0);
Ball ball = createBall();

int main(int argc, const char * argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "Unable to init SDL" << std::endl;
        exit(1);
    }
    
    gWindow = SDL_CreateWindow("Pong",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               WINDOW_WIDTH,
                               WINDOW_HEIGHT,
                               SDL_WINDOW_FULLSCREEN);
    
    if (gWindow == NULL)
    {
        std::cout << "Unable to create window" << std::endl;
        SDL_Quit();
        exit(1);
    }
    
    gRenderer = SDL_CreateRenderer(gWindow,
                                   -1,
                                   SDL_RENDERER_ACCELERATED |
                                   SDL_RENDERER_PRESENTVSYNC);
    
    if (gRenderer == NULL)
    {
        std::cout << "Unable to create renderer" << std::endl;
        SDL_Quit();
        exit(1);
    }
    
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    
    bool running = true;
    SDL_Event event;
    
    double previous = (double)SDL_GetTicks();
    double lag = 0.0;
    
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    running = false;
                    break;
                default:
                    break;
            }
        }
        
        double current = (double)SDL_GetTicks();
        double elapsed = current - previous;
        lag += elapsed;
        previous = current;
        
        while (lag >= MS_PER_UPDATE)
        {
            update();
            lag -= MS_PER_UPDATE;
        }
        
        render(gRenderer);
    }
    
    render(gRenderer);
    
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gRenderer = NULL;
    gWindow = NULL;
    
    SDL_Quit();
    
    return 0;
}

static Paddle createPaddle(PaddlePosition pPosition, int score)
{
    Paddle paddle;
    
    paddle.paddlePosition = pPosition;
    
    paddle.screenPosition = {
        (pPosition == PaddlePosition_Left) ? 20 : WINDOW_WIDTH - 20,
        WINDOW_HEIGHT / 2
    };
    
    paddle.rect = {
        paddle.screenPosition.x - PADDLE_WIDTH / 2,
        paddle.screenPosition.y - PADDLE_HEIGHT / 2,
        PADDLE_WIDTH,
        PADDLE_HEIGHT
    };
    
    paddle.score = score;
    
    return paddle;
}

static Ball createBall()
{
    Ball ball;
    
    ball.screenPosition = {
        WINDOW_WIDTH / 2,
        WINDOW_HEIGHT / 2
    };
    
    ball.rect = {
        ball.screenPosition.x - BALL_RADIUS,
        ball.screenPosition.y - BALL_RADIUS,
        BALL_RADIUS * 2,
        BALL_RADIUS * 2
    };
    
    ball.velocity = {
        -10,
        -10
    };
    
    return ball;
}

const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

static void update()
{
    updateControls();
    updatePaddle(paddle1);
    updatePaddle(paddle2);
    updateBall(ball);
    updateState();
}

static void updatePaddle(Paddle &paddle)
{
    paddle.rect = {
        paddle.screenPosition.x - PADDLE_WIDTH / 2,
        paddle.screenPosition.y - PADDLE_HEIGHT / 2,
        PADDLE_WIDTH,
        PADDLE_HEIGHT
    };
}

static void updateBall(Ball &ball)
{
    ball.screenPosition.x += ball.velocity.x;
    ball.screenPosition.y += ball.velocity.y;
    
    ball.rect = {
        ball.screenPosition.x - BALL_RADIUS,
        ball.screenPosition.y - BALL_RADIUS,
        BALL_RADIUS * 2,
        BALL_RADIUS * 2
    };
    
    keepBallInBounds(ball);
    collideBallWithPaddle(ball, paddle1);
    collideBallWithPaddle(ball, paddle2);
    
    ball.rect = {
        ball.screenPosition.x - BALL_RADIUS,
        ball.screenPosition.y - BALL_RADIUS,
        BALL_RADIUS * 2,
        BALL_RADIUS * 2
    };
}

static void updateControls()
{
    automatePaddle(paddle1);
    automatePaddle(paddle2);
}

static void updateState()
{
    if (ball.rect.x + ball.rect.w < 0 || ball.rect.x > WINDOW_WIDTH)
    {
        if (ball.rect.x + ball.rect.w < 0)
        {
            if (paddle1.paddlePosition == PaddlePosition_Left)
            {
                paddle1.score++;
            }
            
            if (paddle2.paddlePosition == PaddlePosition_Left)
            {
                paddle2.score++;
            }
        }
        
        if (ball.rect.x > WINDOW_WIDTH)
        {
            if (paddle1.paddlePosition == PaddlePosition_Right)
            {
                paddle1.score++;
            }
            
            if (paddle2.paddlePosition == PaddlePosition_Right)
            {
                paddle2.score++;
            }
        }
        
        reset();
    }
}

static void keepBallInBounds(Ball &ball)
{
    if (ball.screenPosition.y < 0)
    {
        ball.screenPosition.y = 0;
        ball.velocity.y *= -1;
    }
    
    if (ball.screenPosition.y > WINDOW_HEIGHT)
    {
        ball.screenPosition.y = WINDOW_HEIGHT;
        ball.velocity.y *= -1;
    }
}

static void collideBallWithPaddle(Ball &ball, Paddle &paddle)
{
    int pLeft = paddle.rect.x;
    int pRight = paddle.rect.x + paddle.rect.w;
    int pTop = paddle.rect.y;
    int pBottom = paddle.rect.y + paddle.rect.h;
    
    int bLeft = ball.rect.x;
    int bRight = ball.rect.x + ball.rect.w;
    int bTop = ball.rect.y;
    int bBottom = ball.rect.y + ball.rect.h;
    
    if (!(bLeft > pRight || bRight < pLeft ||
          bTop > pBottom || bBottom < pTop))
    {
        int lOffset = pRight - bLeft;
        int rOffset = pLeft - bRight;
        int tOffset = pBottom - bTop;
        int bOffset = pTop - bBottom;
        
        int xOffset = (abs(lOffset) < abs(rOffset)) ? lOffset : rOffset;
        int yOffset = (abs(tOffset) < abs(bOffset)) ? tOffset : bOffset;
        
        if (abs(xOffset) < abs(yOffset))
        {
            ball.screenPosition.x += xOffset;
            ball.velocity.x *= -1;
        }
        else
        {
            ball.screenPosition.y += yOffset + getIntSign(yOffset);
            ball.velocity.y *= -1;
        }
    }
}

static void render(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    renderPaddle(renderer, paddle1);
    renderPaddle(renderer, paddle2);
    renderBall(renderer, ball);
    
    SDL_RenderPresent(renderer);
}

static void renderPaddle(SDL_Renderer *renderer, Paddle paddle)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &paddle.rect);
}

static void renderBall(SDL_Renderer *renderer, Ball ball)
{
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderFillRect(renderer, &ball.rect);
}

static void reset()
{
    paddle1 = createPaddle(PaddlePosition_Left, paddle1.score);
    paddle2 = createPaddle(PaddlePosition_Right, paddle2.score);
    ball = createBall();
}

static int getIntSign(int num)
{
    return (num >= 0) ? 1 : -1;
}

static Vector2i getBallPosAtBounce(Ball &ball)
{
    Vector2i position;
    
    if (ball.velocity.x > 0)
    {
        position.x = WINDOW_WIDTH;
    }
    else
    {
        position.x = 0;
    }
    
    if (ball.velocity.y > 0)
    {
        position.y = WINDOW_HEIGHT;
    }
    else
    {
        position.y = 0;
    }
    
    float m;
    
    if (ball.velocity.x != 0)
    {
        m = (float)ball.velocity.y / (float)ball.velocity.x;
    }
    else
    {
        m = (float)ball.velocity.y / 0.00000001;
    }
    
    int y = m * (position.x - ball.screenPosition.x);
    position.y = ball.screenPosition.y + y;
    
    if (position.y > WINDOW_HEIGHT)
    {
        position.y = WINDOW_HEIGHT - (position.y - WINDOW_HEIGHT);
    }
    else if (position.y < 0)
    {
        position.y = -position.y;
    }
    
    return position;
}

static void movePaddleTo(Paddle &paddle, Vector2i position)
{
    int xDirection = (paddle.screenPosition.x < position.x) ? 1 : -1;
    int yDirection = (paddle.screenPosition.y < position.y) ? 1 : -1;
    
    if (paddle.screenPosition.x - PADDLE_SPEED > position.x ||
        paddle.screenPosition.x + PADDLE_SPEED < position.x)
    {
        paddle.screenPosition = {
            paddle.screenPosition.x + PADDLE_SPEED * xDirection,
            paddle.screenPosition.y
        };
    }
    else if (paddle.screenPosition.y - PADDLE_SPEED > position.y ||
             paddle.screenPosition.y + PADDLE_SPEED < position.y)
    {
        paddle.screenPosition = {
            paddle.screenPosition.x,
            paddle.screenPosition.y + PADDLE_SPEED * yDirection
        };
    }
    else if (paddle.screenPosition.x - PADDLE_SPEED > position.x ||
             paddle.screenPosition.x + PADDLE_SPEED < position.x ||
             paddle.screenPosition.y - PADDLE_SPEED > position.y ||
             paddle.screenPosition.y + PADDLE_SPEED < position.y)
    {
        paddle.screenPosition = {
            paddle.screenPosition.x + PADDLE_SPEED * xDirection,
            paddle.screenPosition.y + PADDLE_SPEED * yDirection
        };
    }
}

static void automatePaddle(Paddle &paddle)
{
    Vector2i nextPaddlePosition = {
        paddle.screenPosition.x,
        paddle.screenPosition.y
    };
    Vector2i ballPosAtBounce = getBallPosAtBounce(ball);
    
    if (ball.velocity.x > 0 && paddle.paddlePosition == PaddlePosition_Right)
    {
        nextPaddlePosition = {
            paddle.screenPosition.x,
            ballPosAtBounce.y
        };
    }
    else if (ball.velocity.x < 0 && paddle.paddlePosition == PaddlePosition_Left)
    {
        nextPaddlePosition = {
            paddle.screenPosition.x,
            ballPosAtBounce.y
        };
    }
    
    movePaddleTo(paddle, nextPaddlePosition);
}

static void controlPaddle(Paddle &paddle)
{
    if (keyboardState[SDL_SCANCODE_UP])
    {
        paddle.screenPosition.y -= PADDLE_SPEED;
    }
    
    if (keyboardState[SDL_SCANCODE_DOWN])
    {
        paddle.screenPosition.y += PADDLE_SPEED;
    }
}
