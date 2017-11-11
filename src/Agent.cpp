#include "Agent.h"

Agent::Agent() : sprite_texture(0),
                 position(Vector2D(100, 100)),
	             target(Vector2D(1000, 100)),
	             velocity(Vector2D(0,0)),
	             mass(0.1f),
	             max_force(150),
	             max_velocity(200),
	             orientation(0),
	             color({ 255,255,255,255 }),
				 sprite_num_frames(0),
	             sprite_w(0),
	             sprite_h(0),
	             draw_sprite(false)
{
	steering_behavior = new SteeringBehavior;
}

Agent::~Agent()
{
	if (sprite_texture)
		SDL_DestroyTexture(sprite_texture);
	if (steering_behavior)
		delete (steering_behavior);
}

vector<Vector2D> Agent::BFS(Vector2D start, Vector2D goal, Graph graph) {
	// Creem la frontera on emmagetzarem tots els nodes que visitem 
	// i la inicialitzem amb la posici� del player = start
	queue<Vector2D> frontier;
	frontier.push(start);

	// Creem l'estructura came_from la qual determina el node anterior del que proveniem per tra�ar el cam� 
	// i com la posici� del player = start no provenia de res = NULL ja que aquesta �s la posici� inicial
	map<Vector2D, Vector2D> came_from;
	came_from[start] = NULL;

	vector<Vector2D> path, neighbors;
	Vector2D current, next;
	bool visited;

	// Mentre la frontera no estigui buida, �s a dir, mentre faltin nodes per explorar
	while (!frontier.empty()) {

		// El node actual del qual visitarem els veins es el primer node que hem afegit o el node m�s antic
		// i fem pop ja que en l'iteraci� anterior ja hem fet servir aquest node
		current = frontier.front();
		frontier.pop();

		// Si el node actual �s el node goal, �s a dir, el node de la moneda
		// suem del algoritme i ens decidim a retornar el cam� per arribar-hi
		if (current == goal) {
			// Fem push_back del current el qual = node final o goal 
			// (perqu� volem tra�ar el cam� desde el final al principi)
			path.push_back(current);
			// Mentre no agafem el node incial
			while (current != start) {
				// Anem afegint els nodes que hem visitat abans del node final
				current = came_from[current];
				path.push_back(current);
			}
			// Com que no hem agafat el node incial ja que la condici� no ens ho permetia 
			// (perqu� abans del start no hi ha m�s nodes) li fem un push del start
			path.push_back(start);
			// I com hem tra�at el cam� del final al principi fem un reverse
			// perqu� el personatge comenci del node inicial i vagi fins el node final
			std::reverse(path.begin(), path.end());
			// Per tant, retornem el cam�
			return path;
		}

		// En cas que no haguem trobat el node final, agafem els veins de 0-4 del current
		neighbors = graph.GetConnections(current);

		// Iterem sobre aquest i anem agafant-los un a un
		for (int i = 0; i < neighbors.size(); i++) {
			visited = false;
			next = neighbors[i];

			// Per cadascun determinem si l'hem visitat o no
			for (int j = 0; j < came_from.size(); j++) {
				if (came_from.find(next) != came_from.end()) {
					visited = true;
				}
			}
			// Si no l'hem visitat l'afegim a la frontier
			// i agafem el node current per tra�ar el cam� 
			// (ja que no podem tra�ar un cam� a base de ve�ns)
			if (!visited) {
				came_from[next] = current;
				frontier.push(next);
			}
		}
	}
}

SteeringBehavior * Agent::Behavior()
{
	return steering_behavior;
}

Vector2D Agent::getPosition()
{
	return position;
}

Vector2D Agent::getTarget()
{
	return target;
}

Vector2D Agent::getVelocity()
{
	return velocity;
}

float Agent::getMaxVelocity()
{
	return max_velocity;
}

void Agent::setPosition(Vector2D _position)
{
	position = _position;
}

void Agent::setTarget(Vector2D _target)
{
	target = _target;
}

void Agent::setVelocity(Vector2D _velocity)
{
	velocity = _velocity;
}

void Agent::setMass(float _mass)
{
	mass = _mass;
}

void Agent::setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	color = { r, g, b, a };
}

void Agent::update(Vector2D steering_force, float dtime, SDL_Event *event)
{

	//cout << "agent update:" << endl;

	switch (event->type) {
		/* Keyboard & Mouse events */
	case SDL_KEYDOWN:
		if (event->key.keysym.scancode == SDL_SCANCODE_SPACE)
			draw_sprite = !draw_sprite;
		break;
	default:
		break;
	}


	Vector2D acceleration = steering_force / mass;
	velocity = velocity + acceleration * dtime;
	velocity = velocity.Truncate(max_velocity);

	position = position + velocity * dtime;


	// Update orientation
	if (velocity.Length()>0)
		orientation = (float)(atan2(velocity.y, velocity.x) * RAD2DEG);


	// Trim position values to window size
	if (position.x < 0) position.x = TheApp::Instance()->getWinSize().x;
	if (position.y < 0) position.y = TheApp::Instance()->getWinSize().y;
	if (position.x > TheApp::Instance()->getWinSize().x) position.x = 0;
	if (position.y > TheApp::Instance()->getWinSize().y) position.y = 0;
}

void Agent::draw()
{
	if (draw_sprite)
	{
		Uint32 sprite;
		
		if (velocity.Length() < 5.0)
			sprite = 1;
		else
			sprite = (int)(SDL_GetTicks() / (max_velocity)) % sprite_num_frames;
		
		SDL_Rect srcrect = { (int)sprite * sprite_w, 0, sprite_w, sprite_h };
		SDL_Rect dstrect = { (int)position.x - (sprite_w / 2), (int)position.y - (sprite_h / 2), sprite_w, sprite_h };
		SDL_Point center = { sprite_w / 2, sprite_h / 2 };
		SDL_RenderCopyEx(TheApp::Instance()->getRenderer(), sprite_texture, &srcrect, &dstrect, orientation+90, &center, SDL_FLIP_NONE);
	}
	else 
	{
		draw_circle(TheApp::Instance()->getRenderer(), (int)position.x, (int)position.y, 15, color.r, color.g, color.b, color.a);
		SDL_RenderDrawLine(TheApp::Instance()->getRenderer(), (int)position.x, (int)position.y, (int)(position.x+15*cos(orientation*DEG2RAD)), (int)(position.y+15*sin(orientation*DEG2RAD)));
	}
}

bool Agent::loadSpriteTexture(char* filename, int _num_frames)
{
	if (_num_frames < 1) return false;

	SDL_Surface *image = IMG_Load(filename);
	if (!image) {
		cout << "IMG_Load: " << IMG_GetError() << endl;
		return false;
	}
	sprite_texture = SDL_CreateTextureFromSurface(TheApp::Instance()->getRenderer(), image);

	sprite_num_frames = _num_frames;
	sprite_w = image->w / sprite_num_frames;
	sprite_h = image->h;
	draw_sprite = true;

	if (image)
		SDL_FreeSurface(image);

	return true;
}

