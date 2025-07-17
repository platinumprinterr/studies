import pygame
import time
import random
import pydoc

# Game initialization
width = 1280
height = 720
size = (width, height)
screen = pygame.display.set_mode(size)
clock = pygame.time.Clock()
pygame.init()
fps = 60
done = False
mode = "welcome"
score = [0, 0]
maxscore = 11
p1_width = 30
p1_height = 180
p1x = 50
p1y = height / 2 - p1_height / 2
p1v = 0
p2_width = 30
p2_height = 180
p2x = width - 50 - p2_width
p2y = (height / 2) - (p2_height / 2)
p2v = 0
pspeed = 2
aispeed = 1.5
ballx = width / 2
bally = height / 2
ballSize = 14
ballxv = 0
ballyv = 0
ballDirRight = True
ballSpeed = 8
ballMaxSpeed = 11
font = pygame.font.SysFont("gillsanusltracondensed", 65)
menu_font = pygame.font.SysFont("gillsanusltracondensed", 100)
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
RED = (209, 41, 41)
ball_hit = False


def drawGame():
    """
    Draws the game elements such as borders, lines, player models, and the score table.
    """
    pygame.draw.rect(screen, WHITE, [0, 0, width, height], 10)
    pygame.draw.line(screen, WHITE, [width / 2, 0], [width / 2, height], 10)
    p1_model = pygame.draw.rect(screen, WHITE, [p1x, p1y, p1_width, p1_height])
    p2_model = pygame.draw.rect(screen, WHITE, [p2x, p2y, p2_width, p2_height])

    # Render score and where it's placed
    text = font.render(str(score[0]), True, WHITE)
    screen.blit(text, (width / 2 - (text.get_rect().width + 50), 20))
    screen.blit(font.render(str(score[1]), True, WHITE), ((width / 2) + 50, 20))


def tickPlayers(p1y, p1v):
    """
    Handles player 1 movement based on keyboard input.

    Args:
        p1y (float): The current y position of player 1's paddle.
        p1v (float): The current velocity of player 1's paddle.

    Returns:
        tuple: Updated y position and velocity of player 1's paddle.
    """
    key = pygame.key.get_pressed()
    if key[pygame.K_UP]:
        p1v -= pspeed
    elif key[pygame.K_DOWN]:
        p1v += pspeed

    # Velocity slightly lowered to simulate friction
    p1v = 0.9 * p1v
    p1y += p1v
    if p1y < 0 or p1y + p1_height > height:
        p1y -= p1v
        p1v = 0

    return p1y, p1v


def move_computer(p2y, ballx, bally, ballxv, ballyv):
    """
    Moves the AI-controlled paddle towards the predicted ball position.

    Args:
        p2y (float): Current y position of the paddle.
        ballx (float): Current x position of the ball.
        bally (float): Current y position of the ball.
        ballxv (float): x velocity of the ball.
        ballyv (float): y velocity of the ball.

    Returns:
        float: New y position of the paddle.
    """
    # Predict the ball's future position
    if ballxv > 0:
        predicted_bally = bally + ((width - ballx - p2_width) / ballxv) * ballyv
        if predicted_bally > height or predicted_bally < 0:
            predicted_bally = (
                height - abs(predicted_bally) % height
                if predicted_bally > height
                else abs(predicted_bally) % height
            )
    else:
        predicted_bally = bally

    # Add randomness to AI's movement
    predicted_bally += random.uniform(-30, 30)

    # Move the paddle towards the predicted position
    if predicted_bally > p2y + p2_height / 2:
        p2y += aispeed
    elif predicted_bally < p2y + p2_height / 2:
        p2y -= aispeed

    # Ensure the paddle doesn't go out of bounds
    p2y = max(0, min(height - p2_height, p2y))
    return p2y


def tickBall(ballx, bally, ballxv, ballyv):
    """
    Handles the movement and collision of the ball.

    Args:
        ballx (float): Current x position of the ball.
        bally (float): Current y position of the ball.
        ballxv (float): x velocity of the ball.
        ballyv (float): y velocity of the ball.

    Returns:
        tuple: Updated position and velocity of the ball.
    """
    global ball_hit

    ballx += ballxv
    bally += ballyv
    player1 = [p1x, p1y, p1_width, p1_height]
    player2 = [p2x, p2y, p2_width, p2_height]

    # Collision with player detection
    if ball.colliderect(player1):
        ball_hit = True
        ballx -= ballxv - 10
        ballxv *= -1
        ballyv += random.choice([1, 2])

    if ball.colliderect(player2):
        ballx -= ballxv + 10
        ballxv *= -1
        ballyv += random.choice([1, 2])

    # Boundary collision detection
    if bally <= ballSize:
        bally = ballSize + 1
        ballyv *= -1

    if bally >= height - ballSize:
        bally = height - ballSize - 1
        ballyv *= -1

    # Speed limitation
    if abs(ballxv) > ballMaxSpeed:
        ballxv *= 0.9
        pygame.draw.circle(screen, RED, (ballx, bally), ballSize)

    if abs(ballyv) > ballMaxSpeed:
        ballyv *= 0.9
        pygame.draw.circle(screen, RED, (ballx, bally), ballSize)

    return ballx, bally, ballxv, ballyv


def resetBall():
    """
    Resets the ball to the center of the field and initializes its speed and direction.

    Returns:
        tuple: Initial values for the ball's position, velocity, direction, and paddle positions.
    """
    global ball_hit
    ball_hit = False
    return (
        width / 2,
        height / 2,
        ballSpeed if ballDirRight else -ballSpeed,
        0,
        not ballDirRight,
        height / 2 - p1_height / 2,
        height / 2 - p2_height / 2,
    )


def checkScore():
    """
    Checks if the ball has crossed the field boundaries to determine if there was a score.

    Returns:
        int: 0 if player 1 scored, 1 if player 2 scored, -1 if no score.
    """
    if ballx - ballSize > width:
        return 0
    elif ballx + ballSize < 0:
        return 1
    return -1


def show_welcome():
    """
    Displays the welcome screen.
    """
    screen.fill(BLACK)
    title_text = menu_font.render("Welcome to Simple Pong Game!", True, WHITE)
    instruction_text = font.render("-- > Press any key to continue <--", True, WHITE)
    screen.blit(
        title_text, (width / 2 - title_text.get_rect().width / 2, height / 2 - 50)
    )
    screen.blit(
        instruction_text,
        (width / 2 - instruction_text.get_rect().width / 2, height / 2 + 50),
    )
    pygame.display.update()


def show_goodbye():
    """
    Displays the goodbye screen.
    """
    screen.fill(BLACK)
    title_text = menu_font.render("Thanks for playing, goodbye!", True, WHITE)
    screen.blit(title_text, (width / 2 - title_text.get_rect().width / 2, height / 2))
    pygame.display.update()
    time.sleep(2)  # Display the goodbye screen for 2 seconds
    pygame.quit()
    quit()


# shows menu
def show_main_menu():
    """
    Displays the main menu screen.
    """
    screen.fill(BLACK)
    title_text = menu_font.render("Pong Game", True, WHITE)
    single_player_text = font.render(
        "--> Single Player Mode (Press 1) <--", True, WHITE
    )
    two_player_text = font.render("--> Two Player Mode (Press 2) <--", True, WHITE)
    screen.blit(title_text, (width / 2 - title_text.get_rect().width / 2, height / 4))
    screen.blit(
        single_player_text,
        (width / 2 - single_player_text.get_rect().width / 2, height / 2 - 50),
    )
    screen.blit(
        two_player_text, (width / 2 - two_player_text.get_rect().width / 2, height / 2)
    )
    pygame.display.update()


def show_difficulty_menu():
    """
    Displays the difficulty selection menu for single-player mode.
    """
    screen.fill(BLACK)
    title_text = menu_font.render("Select Difficulty", True, WHITE)
    easy_text = font.render("Easy (Press 1)", True, WHITE)
    medium_text = font.render("Medium (Press 2)", True, WHITE)
    hard_text = font.render("Hard (Press 3)", True, WHITE)
    screen.blit(title_text, (width / 2 - title_text.get_rect().width / 2, height / 4))
    screen.blit(
        easy_text, (width / 2 - easy_text.get_rect().width / 2, height / 2 - 50)
    )
    screen.blit(medium_text, (width / 2 - medium_text.get_rect().width / 2, height / 2))
    screen.blit(
        hard_text, (width / 2 - hard_text.get_rect().width / 2, height / 2 + 50)
    )
    pygame.display.update()


def show_pause_menu():
    """
    Displays the pause menu.
    """
    screen.fill(BLACK)
    title_text = menu_font.render("Game Paused!", True, WHITE)
    resume_text = font.render("Resume (Press R)", True, WHITE)
    menu_text = font.render("Main Menu (Press M)", True, WHITE)
    quit_text = font.render("Quit (Press Q)", True, WHITE)
    screen.blit(title_text, (width / 2 - title_text.get_rect().width / 2, height / 4))
    screen.blit(
        resume_text, (width / 2 - resume_text.get_rect().width / 2, height / 2 - 50)
    )
    screen.blit(menu_text, (width / 2 - menu_text.get_rect().width / 2, height / 2))
    screen.blit(
        quit_text, (width / 2 - quit_text.get_rect().width / 2, height / 2 + 50)
    )
    pygame.display.update()


def return_to_menu():
    """
    Resets the game state and returns to the main menu.
    """
    global mode, score, p1y, p2y
    mode = "main_menu"
    score = [0, 0]
    p1y = height / 2 - p1_height / 2
    p2y = height / 2 - p2_height / 2


def show_game_over(winner, score_diff):
    """
    Displays the game over screen with the winner and score difference.

    Args:
        winner (str): The winner of the game.
        score_diff (int): The score difference.
    """
    screen.fill(BLACK)
    game_over_text = font.render(
        f"Congratulations {winner}! You won the game by {score_diff} points.",
        True,
        WHITE,
    )
    play_again_text = font.render("Press R to play again or ESC to quit.", True, WHITE)
    screen.blit(
        game_over_text,
        (width / 2 - game_over_text.get_rect().width / 2, height / 2 - 50),
    )
    screen.blit(
        play_again_text,
        (width / 2 - play_again_text.get_rect().width / 2, height / 2 + 50),
    )
    pygame.display.update()


ballx, bally, ballxv, ballyv, ballDirRight, p1y, p2y = resetBall()

# GAME LOOP
while not done:
    screen.fill(BLACK)
    for event in pygame.event.get():  # Handles key events for different modes
        if event.type == pygame.QUIT:
            mode = "goodbye"
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE and mode == "game":
                mode = "pause"
            elif event.key == pygame.K_r and mode == "pause":
                mode = "game"
            elif event.key == pygame.K_m and mode == "pause":
                return_to_menu()
            elif event.key == pygame.K_q and mode == "pause":
                mode = "goodbye"

            if mode == "welcome":
                mode = "main_menu"
            elif mode == "main_menu":
                if event.key == pygame.K_1:
                    mode = "difficulty_menu"
                elif event.key == pygame.K_2:
                    score = [0, 0]  # Reset score
                    p1y = height / 2 - p1_height / 2  # Reset player 1 position
                    p2y = height / 2 - p2_height / 2  # Reset player 2 position
                    mode = "game"
                    ai_mode = False
            elif mode == "difficulty_menu":
                if event.key == pygame.K_1:
                    aispeed = 1  # Easy
                    score = [0, 0]
                    p1y = height / 2 - p1_height / 2
                    p2y = height / 2 - p2_height / 2
                    mode = "game"
                    ai_mode = True
                elif event.key == pygame.K_2:
                    aispeed = 1.5  # Medium
                    score = [0, 0]
                    p1y = height / 2 - p1_height / 2
                    p2y = height / 2 - p2_height / 2
                    mode = "game"
                    ai_mode = True
                elif event.key == pygame.K_3:
                    aispeed = 2  # Hard
                    score = [0, 0]
                    p1y = height / 2 - p1_height / 2
                    p2y = height / 2 - p2_height / 2
                    mode = "game"
                    ai_mode = True
            elif mode == "game over":
                if event.key == pygame.K_r:
                    score = [0, 0]
                    mode = "main_menu"  # Go back to main menu
                elif event.key == pygame.K_ESCAPE:
                    mode = "goodbye"
    # Handles different modes and functions linked to them
    if mode == "welcome":
        show_welcome()

    if mode == "main_menu":
        show_main_menu()

    if mode == "difficulty_menu":
        show_difficulty_menu()

    if mode == "game":
        if checkScore() != -1:  # Checks if anyone scored
            mode = "scored"
            score[checkScore()] += 1
            timer = 0
            ballx, bally, ballxv, ballyv, ballDirRight, p1y, p2y = resetBall()

        drawGame()
        ball = pygame.draw.circle(screen, WHITE, (ballx, bally), ballSize)
        p1y, p1v = tickPlayers(p1y, p1v)

        if ai_mode:
            # Update player 2 position with AI only after player 1 hits the ball
            if ball_hit:
                p2y = move_computer(p2y, ballx, bally, ballxv, ballyv)
        else:
            # Update player 2 position with human controls
            key = pygame.key.get_pressed()
            if key[pygame.K_w]:
                p2v -= pspeed
            elif key[pygame.K_s]:
                p2v += pspeed
            p2y += p2v
            p2v = 0.9 * p2v
            if p2y < 0 or p2y + p2_height > height:
                p2y -= p2v
                p2v = 0

        ballx, bally, ballxv, ballyv = tickBall(ballx, bally, ballxv, ballyv)
        # Checks if either player reached max score to end the game
        if score[0] >= maxscore or score[1] >= maxscore:
            winner = "player 1" if score[0] > score[1] else "player 2"
            score_diff = abs(score[0] - score[1])
            mode = "game over"
            show_game_over(winner, score_diff)

        pygame.display.update()

    if mode == "scored":
        timer += 1
        if timer >= 1.5 * fps:
            mode = "game"

    if mode == "game over":
        # Ensure game over screen is displayed and handles key events
        show_game_over(winner, score_diff)
        pygame.display.update()

    if mode == "goodbye":
        show_goodbye()

    if mode == "pause":
        show_pause_menu()

    clock.tick(fps)
