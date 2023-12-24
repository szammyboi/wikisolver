#include <SFML/Graphics.hpp>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <sstream>

namespace fs = std::filesystem;
using namespace std::chrono;

// Placeholder function to represent Dijkstra's algorithm.
void runDijkstrasAlgorithm() {
    // Your algorithm's implementation would go here
    // Simulating a delay for demonstration purposes
    std::this_thread::sleep_for(milliseconds(4300));
}

// Placeholder function to represent the A* algorithm.
void runAStarAlgorithm() {
    // Your algorithm's implementation would go here
    // Simulating a delay for demonstration purposes
    std::this_thread::sleep_for(milliseconds(3200));
}

int main() {
    sf::RenderWindow window(sf::VideoMode(640, 800), "Wikipedia Solver");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("fonts/arial.ttf")) {
        std::cerr << "Failed to load fonts/arial.ttf" << std::endl;
        return EXIT_FAILURE;
    }

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("texture/engin-akyurt-epgqiRNfNHw-unsplash.jpg")) {
        std::cerr << "Failed to load texture/engin-akyurt-epgqiRNfNHw-unsplash.jpg" << std::endl;
        return EXIT_FAILURE;
    }
    sf::Sprite backgroundSprite(backgroundTexture);
    backgroundSprite.setScale(
        float(window.getSize().x) / backgroundTexture.getSize().x,
        float(window.getSize().y) / backgroundTexture.getSize().y
    );

    sf::RectangleShape leftPanel(sf::Vector2f(325, 750));
    leftPanel.setPosition(0, 100);
    leftPanel.setOutlineThickness(1);
    leftPanel.setOutlineColor(sf::Color::White);
    leftPanel.setFillColor(sf::Color(50, 50, 50));

    sf::RectangleShape rightPanel = leftPanel;
    rightPanel.setPosition(320, 100);

    sf::Text title("WIKIPEDIA SOLVER", font, 50);
    title.setPosition(window.getSize().x / 2 - title.getGlobalBounds().width / 2, 10);
    title.setFillColor(sf::Color::White);

    sf::Text startLabel("START PAGE", font, 20);
    startLabel.setPosition(10, 60);
    startLabel.setFillColor(sf::Color::White);

    sf::Text toLabel("TO", font, 20);
    toLabel.setPosition(window.getSize().x / 2 - toLabel.getLocalBounds().width / 2, 60);
    toLabel.setFillColor(sf::Color::White);

    sf::Text targetLabel("TARGET PAGE", font, 20);
    targetLabel.setPosition(window.getSize().x - targetLabel.getLocalBounds().width - 10, 60);
    targetLabel.setFillColor(sf::Color::White);

    sf::Text dijkstraText("DIJKSTRA'S ALGORITHM", font, 20);
    dijkstraText.setPosition(10, leftPanel.getPosition().y + 10);
    dijkstraText.setFillColor(sf::Color::White);

    sf::Text aStarText("A* ALGORITHM", font, 20);
    aStarText.setPosition(325 + 10, rightPanel.getPosition().y + 10);
    aStarText.setFillColor(sf::Color::White);

    // Create input fields for the URLs
    sf::RectangleShape startInputField(sf::Vector2f(300, 20));
    startInputField.setPosition(0, 65);
    startInputField.setFillColor(sf::Color(255, 255, 255));
    startInputField.setOutlineColor(sf::Color::Black);
    startInputField.setOutlineThickness(1); // Added outline for visibility

    sf::RectangleShape targetInputField(sf::Vector2f(300, 20));
    targetInputField.setPosition(345, 65);
    targetInputField.setFillColor(sf::Color(255, 255, 255));
    targetInputField.setOutlineColor(sf::Color::Black);
    targetInputField.setOutlineThickness(1); // Added outline for visibility

    // Text objects to display URLs
    sf::Text startUrlText("", font, 20);
    startUrlText.setPosition(15, 95);
    startUrlText.setFillColor(sf::Color::Black);

    sf::Text targetUrlText("", font, 20);
    targetUrlText.setPosition(325 + 15, 95);
    targetUrlText.setFillColor(sf::Color::Black);

    // Flags to track if the user is typing in a specific input field
    bool typingStartUrl = false;
    bool typingTargetUrl = false;

    // Strings to hold the URLs
    std::string startUrl = "";
    std::string targetUrl = "";

    // Timing Dijkstra's Algorithm
    auto startDijkstra = high_resolution_clock::now();
    runDijkstrasAlgorithm(); // Replace with your actual Dijkstra's algorithm function
    auto endDijkstra = high_resolution_clock::now();
    auto durationDijkstra = duration<double>(endDijkstra - startDijkstra).count();
    std::ostringstream dijkstraStream;
    dijkstraStream.precision(2);
    dijkstraStream << std::fixed << durationDijkstra;
    sf::Text dijkstraTime("Execution Time: " + dijkstraStream.str() + "s", font, 20);
    dijkstraTime.setPosition(10, dijkstraText.getPosition().y + dijkstraText.getGlobalBounds().height + 5);
    dijkstraTime.setFillColor(sf::Color::White);

    // Timing A* Algorithm
    auto startAStar = high_resolution_clock::now();
    runAStarAlgorithm(); // Replace with your actual A* algorithm function
    auto endAStar = high_resolution_clock::now();
    auto durationAStar = duration<double>(endAStar - startAStar).count();
    std::ostringstream aStarStream;
    aStarStream.precision(2);
    aStarStream << std::fixed << durationAStar;
    sf::Text aStarTime("Execution Time: " + aStarStream.str() + "s", font, 20);
    aStarTime.setPosition(325 + 10, aStarText.getPosition().y + aStarText.getGlobalBounds().height + 5);
    aStarTime.setFillColor(sf::Color::White);

    const int numberOfPages = 8;
    std::vector<sf::Text> dijkstraPages;
    std::vector<sf::Text> aStarPages;
    for (int i = 0; i < numberOfPages; ++i) {
        sf::Text pageText("PAGE " + std::to_string(i + 1), font, 20);
        pageText.setPosition(10, 190 + i * 70);
        pageText.setFillColor(sf::Color::White);
        dijkstraPages.push_back(pageText);

        pageText.setPosition(325 + 10, 190 + i * 70);
        aStarPages.push_back(pageText);
    }

    sf::Texture arrowTexture;
    if (!arrowTexture.loadFromFile("texture/arrow.png")) {
        std::cerr << "Failed to load texture/arrow.png" << std::endl;
        return EXIT_FAILURE;
    }
    std::vector<sf::Sprite> dijkstraArrows;
    for (int i = 0; i < numberOfPages - 1; ++i) {
        sf::Sprite arrowSprite(arrowTexture);
        arrowSprite.setPosition(30, 175 + i * 70 + 40);
        arrowSprite.setScale(0.15f, 0.15f);
        dijkstraArrows.push_back(arrowSprite);
    }

    std::vector<sf::Sprite> aStarArrows;
    for (int i = 0; i < numberOfPages - 1; ++i) {
        sf::Sprite arrowSprite(arrowTexture);
        arrowSprite.setPosition(345, 175 + i * 70 + 40);
        arrowSprite.setScale(0.15f, 0.15f);
        aStarArrows.push_back(arrowSprite);
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::TextEntered:
                if (typingStartUrl) {
                    // Handle backspace
                    if (event.text.unicode == '\b') {
                        if (!startUrl.empty()) {
                            startUrl.pop_back();
                        }
                    }
                    else if (event.text.unicode < 128) {
                        // Add the new character to the URL
                        startUrl += static_cast<char>(event.text.unicode);
                    }
                    // Update the display text
                    startUrlText.setString(startUrl);
                }
                else if (typingTargetUrl) {
                    // Handle backspace
                    if (event.text.unicode == '\b') {
                        if (!targetUrl.empty()) {
                            targetUrl.pop_back();
                        }
                    }
                    else if (event.text.unicode < 128) {
                        // Add the new character to the URL
                        targetUrl += static_cast<char>(event.text.unicode);
                    }
                    // Update the display text
                    targetUrlText.setString(targetUrl);
                }
                break;
            case sf::Event::MouseButtonPressed:
                if (startInputField.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    typingStartUrl = true;
                    typingTargetUrl = false;
                }
                else if (targetInputField.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                    typingStartUrl = false;
                    typingTargetUrl = true;
                }
                else {
                    typingStartUrl = false;
                    typingTargetUrl = false;
                }
                break;
            }
        }

        window.clear();

        // Draw background
        window.draw(backgroundSprite);

        // Draw panels
        window.draw(leftPanel);
        window.draw(rightPanel);

        // Draw titles
        window.draw(title);
        window.draw(startLabel);
        window.draw(toLabel);
        window.draw(targetLabel);
        window.draw(dijkstraText);
        window.draw(aStarText);

        // Draw execution times
        window.draw(dijkstraTime);
        window.draw(aStarTime);

        // Draw input fields
        window.draw(startInputField);
        window.draw(targetInputField);

        // Draw URL text
        window.draw(startUrlText);
        window.draw(targetUrlText);

        // Draw page labels and arrows for both algorithms
        for (auto& pageText : dijkstraPages) {
            window.draw(pageText);
        }
        for (auto& pageText : aStarPages) {
            window.draw(pageText);
        }
        for (auto& arrowSprite : dijkstraArrows) {
            window.draw(arrowSprite);
        }
        for (auto& arrowSprite : aStarArrows) {
            window.draw(arrowSprite);
        }

        window.display();
    }

    return 0;
}