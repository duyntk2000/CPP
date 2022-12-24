
#include <SFML/Graphics.hpp>
#include <math.h>
#include <iostream>

class ParticleSystem : public sf::Drawable, public sf::Transformable
{
public:

    ParticleSystem(unsigned int count, unsigned int r, unsigned int g, unsigned int b, unsigned int a, bool animation, float radius, float size, float speed) :
    m_particles(count),
    m_vertices(sf::Points, count),
    m_lifetime(sf::seconds(2.f)),
    m_time(sf::seconds(15.f)),
    m_emitter(0.f, 0.f),
    m_animation(animation),
    m_radius(radius),
    m_size(size),
    m_speed(speed)
    {
        for (std::size_t i = 0; i < m_particles.size(); ++i)
            m_vertices[i].color = sf::Color(r,g,b,a);
    }

    void setEmitter(sf::Vector2f position)
    {
        m_emitter.x = position.x*m_size+15.f;
        m_emitter.y = position.y*m_size+15.f;
        m_velocity = position;
        //std::cout << m_velocity.x << " " << m_velocity.y << std::endl;
        for (std::size_t i = 0; i < m_particles.size(); ++i)
            {
                resetParticle(i, m_radius);
            }
    }

    void update(sf::Time elapsed, unsigned tick)
    {
        if (m_animation) {
            if (tick % 100 == 99) {
                m_radius += m_speed;
                m_speed = -m_speed;
            }
            for (std::size_t i = 0; i < m_particles.size(); ++i)
            {
                // update the particle lifetime
                Particle& p = m_particles[i];
                p.lifetime -= elapsed;

                // if the particle is dead, respawn it
                if (p.lifetime <= sf::Time::Zero)
                    resetParticle(i, m_radius);

                // update the alpha (transparency) of the particle according to its lifetime
                float ratio = p.lifetime.asSeconds() / m_lifetime.asSeconds();
                m_vertices[i].color.a = static_cast<sf::Uint8>(ratio * 255);
            }
        }
        else {
            if (tick % 100 == 99) {
                m_velocity = -m_velocity;
            }
            for (std::size_t i = 0; i < m_particles.size(); ++i)
            {

                m_vertices[i].position += m_velocity*0.0003f;
            }
        }
    }

private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // apply the transform
        states.transform *= getTransform();

        // our particles don't use a texture
        states.texture = NULL;

        // draw the vertex array
        target.draw(m_vertices, states);
    }

public:

    struct Particle
    {
        sf::Vector2f velocity;
        sf::Time lifetime;
    };

    void resetParticle(std::size_t index, float radius)
    {
        // give a random velocity and lifetime to the particle
        float angle = (std::rand() % 360);
        float speed = std::rand() % 10 + 1;
        m_particles[index].velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        m_particles[index].lifetime = sf::milliseconds(std::rand() % 2500);

        // reset the position of the corresponding vertex
        m_vertices[index].position = m_emitter + m_particles[index].velocity * radius;
    }

    std::vector<Particle> m_particles;
    sf::VertexArray m_vertices;
    sf::Time m_lifetime;
    sf::Vector2f m_emitter;
    sf::Vector2f m_velocity;
    sf::Time m_time;
    bool m_animation;
    float m_radius;
    float m_size;
    float m_speed;
};

std::tuple<std::vector<float>, std::vector<float>> generate_points(const int n_pieces, const float size) {
    std::vector<float> vx, vy;

    constexpr float PI = 3.141592653;
    float dt = 2.0 * PI / n_pieces;

    for (float t = 0.0; t <= 2.0*PI; t += dt) {
        float x = 16.0 * sin(t) * sin(t) * sin(t);
        float y = 13.0 * cos(t) - 5.0 * cos(2.0 * t) - 2.0 * cos(3.0 * t) - cos(4.0 * t);
        //std::cout << "x : " << x << " y : " << y << std::endl;
        //x = x*size+15.0;
        //y = y*-size+15.0;
        //std::cout << "x : " << x << " y : " << y << std::endl;
        vx.emplace_back(x);
        vy.emplace_back(-y);
    }

    return std::make_tuple(vx, vy);
}

int main()
{
    // create the window
    sf::RenderWindow window(sf::VideoMode(600, 600), "Heart");
    sf::View view(sf::Vector2f(15.f,15.f), sf::Vector2f(30.f, 30.f));
    window.setView(view);
    // create the particle system

    std::tuple<std::vector<float>, std::vector<float>> outer = generate_points(80, 0.45);
    std::vector<float> x = std::get<0>(outer);
    std::vector<float> y = std::get<1>(outer);

    std::vector<ParticleSystem> particless;

    for (unsigned i = 0; i < 80; i++) {
        sf::Vector2f v(x[i], y[i]);
        ParticleSystem particle(100,255,240,240,100,true, 0.15f, 0.35f, 0.2f);
        particle.setEmitter(v);
        particless.emplace_back(particle);
    }

    std::vector<std::vector<ParticleSystem>> heart;
    std::tuple<std::vector<float>, std::vector<float>> heart_co = generate_points(110, 0.05+0.035*10);
    std::vector<float> vx = std::get<0>(heart_co);
    std::vector<float> vy = std::get<1>(heart_co);
    for (unsigned n = 0; n < 10; n++) {
        std::vector<ParticleSystem> particles;
        for (unsigned i = 0; i < 110; i++) {
            if (vx[i] <= 1.f && vx[i] >= -1.f) {
                if (n%4!=0)
                    continue;
            }
            sf::Vector2f v(vx[i], vy[i]);
            ParticleSystem particle(1*(n+2)*(n+1),240,103,152,15*(n+1),false,0.08,0.05+0.035*n,0.05*(10-n));
            particle.setEmitter(v);
            particles.emplace_back(particle);
        }
        heart.emplace_back(particles);
    }

    heart.emplace_back(particless);

    // create a clock to track the elapsed time
    sf::Clock clock;
    unsigned tick = 0;
    // run the main loop
    while (window.isOpen())
    {
        // handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }

        // make the particle system emitter follow the mouse
        // sf::Vector2i mouse = sf::Mouse::getPosition(window);
        // particles.setEmitter(window.mapPixelToCoords(mouse));

        // update it
        tick++;
        sf::Time elapsed = clock.restart();
        for (auto& particles : heart)
            for (auto& par : particles) {
                par.update(elapsed,tick);
            }

        // draw it
        window.clear();

        for (auto& particles : heart)
            for (auto& par : particles)
                window.draw(par);

        window.display();
    }

    return 0;
}
