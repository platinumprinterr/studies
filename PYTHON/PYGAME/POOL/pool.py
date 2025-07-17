import pygame
import pymunk
import pymunk.pygame_util
import math

# Define game constants and variables
SCREEN_WIDTH = 1200
SCREEN_HEIGHT = 678
BOTTOM_PANEL = 50
FPS = 120

# Colours
BG = (50, 50, 50)
RED = (255, 0, 0)
WHITE = (255, 255, 255)
GREEN = (0, 255, 0)
DARKGRAY = (47, 79, 79)

# Game variables
lifes = 3
dia = 36
pocket_dia = 66
force = 0
max_force = 10000
force_direction = 1
game_running = False
cue_ball_potted = False
taking_shot = True
powering_up = False
potted_balls = []
balls = []  # Initialize the balls list

# Initialize pygame
pygame.init()

# Initialize font module
pygame.font.init()

# Fonts
font = pygame.font.SysFont("Lato", 30)
large_font = pygame.font.SysFont("Lato", 60)
small_font = pygame.font.SysFont("Lato", 20)

# Create power bars to show how hard the cue ball will be hit
power_bar = pygame.Surface((10, 20))
power_bar.fill(RED)


def init_game():
    """
    Initializes the game window, Pymunk physics space, and other game settings.
    """
    global screen, space, static_body, draw_options, clock
    screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT + BOTTOM_PANEL))
    pygame.display.set_caption("POOL GAME")
    space = pymunk.Space()
    static_body = space.static_body
    draw_options = pymunk.pygame_util.DrawOptions(screen)
    clock = pygame.time.Clock()


def load_images():
    """
    Loads images for the cue, table, and balls from specified file paths.
    """
    global cue_image, table_image, ball_images
    cue_image = pygame.image.load(
        "C:/Users/MINAD/Desktop/FLEGMA/PROGRAMOWANIE/PYTHOn/POOL/assets/images/cue.png"
    ).convert_alpha()
    table_image = pygame.image.load(
        "C:/Users/MINAD/Desktop/FLEGMA/PROGRAMOWANIE/PYTHOn/POOL/assets/images/table.png"
    ).convert_alpha()
    ball_images = []
    for i in range(1, 17):
        ball_image = pygame.image.load(
            f"C:/Users/MINAD/Desktop/FLEGMA/PROGRAMOWANIE/PYTHOn/POOL/assets/images/ball_{i}.png"
        ).convert_alpha()
        ball_images.append(ball_image)


def setup_balls():
    """
    Initializes the balls on the table, including the cue ball, and positions them in a triangular formation.
    """
    global balls, cue_ball
    balls = []  # Initialize the balls list
    rows = 5
    spacing = dia + 2
    for col in range(5):
        for row in range(rows):
            pos = (250 + (col * spacing), 267 + (row * spacing) + (col * dia / 2))
            new_ball = create_ball(dia / 2, pos)
            balls.append(new_ball)
        rows -= 1
    pos = (888, SCREEN_HEIGHT / 2)
    cue_ball = create_ball(dia / 2, pos)
    balls.append(cue_ball)


def setup_cushions():
    """
    Sets up the cushions (bumpers) around the pool table using predefined coordinates.
    """
    cushions = [
        [(88, 56), (109, 77), (555, 77), (564, 56)],
        [(621, 56), (630, 77), (1081, 77), (1102, 56)],
        [(89, 621), (110, 600), (556, 600), (564, 621)],
        [(622, 621), (630, 600), (1081, 600), (1102, 621)],
        [(56, 96), (77, 117), (77, 560), (56, 581)],
        [(1143, 96), (1122, 117), (1122, 560), (1143, 581)],
    ]
    for c in cushions:
        create_cushion(c)


def setup_pockets():
    """
    Defines the positions of the pockets on the pool table.
    """
    global pockets
    pockets = [(55, 63), (592, 48), (1134, 64), (55, 616), (592, 629), (1134, 616)]


def draw_text(text, font, text_col, x, y):
    """
    Renders text on the screen at specified coordinates.

    Parameters:
    text (str): The text to display.
    font (pygame.font.Font): The font to use for rendering.
    text_col (tuple): The color of the text (RGB).
    x (int): The x-coordinate for the text position.
    y (int): The y-coordinate for the text position.
    """
    img = font.render(text, True, text_col)
    screen.blit(img, (x, y))


def create_ball(radius, pos):
    """
    Creates a ball with a specified radius and position, adds it to the physics space, and returns the shape.

    Parameters:
    radius (float): The radius of the ball.
    pos (tuple): The position of the ball (x, y).

    Returns:
    pymunk.Circle: The created ball shape.
    """
    body = pymunk.Body()
    body.position = pos
    shape = pymunk.Circle(body, radius)
    shape.mass = 5
    shape.elasticity = 0.8
    pivot = pymunk.PivotJoint(static_body, body, (0, 0), (0, 0))
    pivot.max_bias = 0
    pivot.max_force = 1000
    space.add(body, shape, pivot)
    return shape


def create_cushion(poly_dims):
    """
    Creates a cushion using polygon dimensions and adds it to the physics space.

    Parameters:
    poly_dims (list): The dimensions of the cushion polygon.
    """
    body = pymunk.Body(body_type=pymunk.Body.STATIC)
    body.position = (0, 0)
    shape = pymunk.Poly(body, poly_dims)
    shape.elasticity = 0.8
    space.add(body, shape)


class Cue:
    """
    Represents the pool cue, including its position and angle.

    Attributes:
    original_image (pygame.Surface): The original image of the cue.
    angle (float): The current angle of the cue.
    image (pygame.Surface): The rotated image of the cue.
    rect (pygame.Rect): The rectangle representing the cue's position.
    """

    def __init__(self, pos):
        """
        Initializes the cue with a specified position.

        Parameters:
        pos (tuple): The initial position of the cue (x, y).
        """
        self.original_image = cue_image
        self.angle = 0
        self.image = pygame.transform.rotate(self.original_image, self.angle)
        self.rect = self.image.get_rect()
        self.rect.center = pos

    def update(self, angle):
        """
        Updates the angle of the cue.

        Parameters:
        angle (float): The new angle of the cue.
        """
        self.angle = angle

    def draw(self, surface):
        """
        Draws the cue on the given surface.

        Parameters:
        surface (pygame.Surface): The surface to draw the cue on.
        """
        self.image = pygame.transform.rotate(self.original_image, self.angle)
        surface.blit(
            self.image,
            (
                self.rect.centerx - self.image.get_width() / 2,
                self.rect.centery - self.image.get_height() / 2,
            ),
        )


def draw_balls():
    """
    Draws all the balls on the screen based on their current positions.
    """
    for i, ball in enumerate(balls):
        screen.blit(
            ball_images[i],
            (ball.body.position[0] - ball.radius, ball.body.position[1] - ball.radius),
        )


def game_menu():
    """
    Displays the main menu and handles user input to start the game or quit.

    Returns:
    bool: True if the game should continue, False if it should quit.
    """
    global game_running, lifes
    menu_running = True
    while menu_running:
        screen.fill(DARKGRAY)
        draw_text(
            "POOL GAME MENU",
            large_font,
            WHITE,
            SCREEN_WIDTH / 2 - 220,
            SCREEN_HEIGHT / 2 - 200,
        )
        draw_text(
            "Press 1 for Easy (5 lifes)",
            font,
            WHITE,
            SCREEN_WIDTH / 2 - 150,
            SCREEN_HEIGHT / 2 - 50,
        )
        draw_text(
            "Press 2 for Normal (3 lifes)",
            font,
            WHITE,
            SCREEN_WIDTH / 2 - 150,
            SCREEN_HEIGHT / 2,
        )
        draw_text(
            "Press 3 for Hardcore (1 life)",
            font,
            WHITE,
            SCREEN_WIDTH / 2 - 150,
            SCREEN_HEIGHT / 2 + 50,
        )
        draw_text(
            "Press Q to Quit",
            font,
            WHITE,
            SCREEN_WIDTH / 2 - 100,
            SCREEN_HEIGHT / 2 + 100,
        )
        draw_text(
            "made by 257188",
            small_font,
            WHITE,
            SCREEN_WIDTH / 2 - 80,
            SCREEN_HEIGHT / 2 + 320,
        )

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                return False
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_1:
                    lifes = 5
                    game_running = True
                    menu_running = False
                if event.key == pygame.K_2:
                    lifes = 3
                    game_running = True
                    menu_running = False
                if event.key == pygame.K_3:
                    lifes = 1
                    game_running = True
                    menu_running = False
                if event.key == pygame.K_q:
                    pygame.quit()
                    return False

        pygame.display.update()

    return True


def pause_menu():
    """
    Displays the pause menu and handles user input to resume or quit the game.
    """
    global game_running
    paused = True
    while paused:
        screen.fill(DARKGRAY)
        draw_text(
            "GAME PAUSED",
            large_font,
            WHITE,
            SCREEN_WIDTH / 2 - 170,
            SCREEN_HEIGHT / 2 - 200,
        )
        draw_text(
            "Press R to Resume",
            font,
            WHITE,
            SCREEN_WIDTH / 2 - 100,
            SCREEN_HEIGHT / 2 - 50,
        )
        draw_text(
            "Press Q to Quit", font, WHITE, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2
        )

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                return False
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_r:
                    game_running = True
                    paused = False
                if event.key == pygame.K_q:
                    pygame.quit()
                    return False

        pygame.display.update()


def reset_game():
    """
    Resets the game state, including lives, force, and the positions of the balls and cushions.
    """
    global \
        lifes, \
        force, \
        force_direction, \
        cue_ball_potted, \
        taking_shot, \
        powering_up, \
        potted_balls, \
        game_running
    force = 0
    force_direction = 1
    cue_ball_potted = False
    taking_shot = True
    powering_up = False
    potted_balls = []
    setup_balls()
    setup_cushions()
    setup_pockets()
    game_running = True


def handle_ball_potting():
    """
    Checks if any balls have been potted and updates the game state accordingly.
    """
    global lifes, cue_ball_potted
    balls_to_remove = []
    for i, ball in enumerate(balls):
        for pocket in pockets:
            ball_x_dist = abs(ball.body.position[0] - pocket[0])
            ball_y_dist = abs(ball.body.position[1] - pocket[1])
            ball_dist = math.sqrt((ball_x_dist**2) + (ball_y_dist**2))
            if ball_dist <= pocket_dia / 2:
                if i == len(balls) - 1:  # Cue ball
                    lifes -= 1
                    cue_ball_potted = True
                    ball.body.position = (-100, -100)
                    ball.body.velocity = (0.0, 0.0)
                else:
                    balls_to_remove.append((ball, i))

    for ball, i in balls_to_remove:
        space.remove(ball.body)
        balls.remove(ball)
        potted_balls.append(ball_images[i])
        ball_images.pop(i)


def check_game_over():
    """
    Checks if the game is over (either by losing all lives or potting all balls) and displays the appropriate message.
    """
    global game_running
    if lifes <= 0:
        draw_text(
            "GAME OVER",
            large_font,
            WHITE,
            SCREEN_WIDTH / 2 - 160,
            SCREEN_HEIGHT / 2 - 100,
        )
        game_running = False
    elif len(balls) == 1:
        draw_text(
            "YOU WIN!",
            large_font,
            WHITE,
            SCREEN_WIDTH / 2 - 160,
            SCREEN_HEIGHT / 2 - 100,
        )
        game_running = False


def handle_cue():
    """
    Updates the position and angle of the cue based on the mouse position and draws it on the screen.
    """
    global cue_ball_potted  # Declare cue_ball_potted as global
    global cue_angle
    if taking_shot and game_running:
        if cue_ball_potted:
            balls[-1].body.position = (888, SCREEN_HEIGHT / 2)
            cue_ball_potted = False
        mouse_pos = pygame.mouse.get_pos()
        cue.rect.center = balls[-1].body.position
        x_dist = balls[-1].body.position[0] - mouse_pos[0]
        y_dist = -(balls[-1].body.position[1] - mouse_pos[1])
        cue_angle = math.degrees(math.atan2(y_dist, x_dist))
        cue.update(cue_angle)
        cue.draw(screen)


def handle_power_up():
    """
    Manages the power-up mechanics for the cue, adjusting the force applied to the cue ball based on user input.
    """
    global force, force_direction
    if powering_up and game_running:
        force += 100 * force_direction
        if force >= max_force or force <= 0:
            force_direction *= -1
        for b in range(math.ceil(force / 2000)):
            screen.blit(
                power_bar,
                (
                    balls[-1].body.position[0] - 30 + (b * 15),
                    balls[-1].body.position[1] + 30,
                ),
            )
    elif not powering_up and taking_shot:
        x_impulse = math.cos(math.radians(cue_angle))
        y_impulse = math.sin(math.radians(cue_angle))
        balls[-1].body.apply_impulse_at_local_point(
            (force * -x_impulse, force * y_impulse), (0, 0)
        )
        force = 0
        force_direction = 1


def handle_events():
    """
    Handles user input events, including mouse clicks and keyboard presses, to control the game flow.
    """
    global powering_up, run
    for event in pygame.event.get():
        if event.type == pygame.MOUSEBUTTONDOWN and taking_shot:
            powering_up = True
        if event.type == pygame.MOUSEBUTTONUP and taking_shot:
            powering_up = False
        if event.type == pygame.QUIT:
            run = False
        if event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE:
            game_running = False
            pause_menu()
            game_running = True


# Initialize the game
init_game()
load_images()
reset_game()

# Game loop
cue = Cue(balls[-1].body.position)
cue_angle = 0  # Initialize cue_angle outside the loop

run = game_menu()
while run:
    clock.tick(FPS)
    space.step(1 / FPS)

    # Fill background
    screen.fill(BG)

    # Draw pool table
    screen.blit(table_image, (0, 0))

    # Handle ball potting
    handle_ball_potting()

    # Draw pool balls
    draw_balls()

    # Check if all the balls have stopped moving
    taking_shot = all(
        int(ball.body.velocity[0]) == 0 and int(ball.body.velocity[1]) == 0
        for ball in balls
    )

    # Handle cue
    handle_cue()

    # Handle power-up
    handle_power_up()

    # Draw bottom panel
    pygame.draw.rect(screen, BG, (0, SCREEN_HEIGHT, SCREEN_WIDTH, BOTTOM_PANEL))
    draw_text(
        "LIFES: " + str(lifes), font, WHITE, SCREEN_WIDTH - 200, SCREEN_HEIGHT + 10
    )

    # Draw potted balls in bottom panel
    for i, ball in enumerate(potted_balls):
        screen.blit(ball, (10 + (i * 50), SCREEN_HEIGHT + 10))

    # Check for game over
    check_game_over()

    # Handle events
    handle_events()

    # space.debug_draw(draw_options)
    pygame.display.update()

pygame.quit()
