#include "model.h"

#define POLYNOME 0x0B
#define QUEUE_SIZE 100
#define MESSAGE_LENGTH 4
#define DIRECT_ERROR 0.4
#define ZERO_ERROR 0.0
#define DEFAULT_TAU 2.0

void firstEx(long messages[]);
void secondEx(long messages[]);
void thirdEx(long messages[]);
void thirdExBorder(long messages[]);
void fourthEx(long messages[]);

int main() {
	long messages[QUEUE_SIZE];
	srand(1);
	long maxVal = 1 << 7;

	for (size_t i = 0; i < QUEUE_SIZE; ++i) {
		messages[i] = rand() % maxVal;
	}

	//long messages[] = { 0x0D, 0x01, 0x0C, 0x05, 0x04, 0x01, 0x05, 0x0A, 0x01, 0x08};

	//firstEx(messages);
	//secondEx(messages);
	//thirdEx(messages);
	//thirdExBorder(messages);
	fourthEx(messages);
}

// Deafult params
void firstEx(long messages[]) {
	Model model(POLYNOME, DIRECT_ERROR, ZERO_ERROR, MESSAGE_LENGTH);

	model.runModel(messages, QUEUE_SIZE, 0, DEFAULT_TAU);
}

// Attempts border
void secondEx(long messages[]) {
	Model model(POLYNOME, DIRECT_ERROR, ZERO_ERROR, MESSAGE_LENGTH);

	model.runModel(messages, QUEUE_SIZE, 2, DEFAULT_TAU);
}

// Return channel error
void thirdEx(long messages[]) {
	Model model(POLYNOME, DIRECT_ERROR, 0.4, MESSAGE_LENGTH);

	model.runModel(messages, QUEUE_SIZE, 0, DEFAULT_TAU);
}

// Return channel error with attempts border
void thirdExBorder(long messages[]) {
	Model model(POLYNOME, DIRECT_ERROR, 0.4, MESSAGE_LENGTH);

	model.runModel(messages, QUEUE_SIZE, 2, DEFAULT_TAU);
}

// Tau requests
void fourthEx(long messages[]) {
	double tau = 2;

	Model model(POLYNOME, 0.0, ZERO_ERROR, MESSAGE_LENGTH);
	model.logOff();

	std::cout << "utilisation = [" << std::endl;
	for (double directError = 0.0; directError < 1.0; directError += 0.01) {
		model.setDirectError(directError);
		std::cout << model.runModel(messages, QUEUE_SIZE, 0, tau);

		if (directError < 0.99)
			std::cout << ",";
		std::cout << std::endl;
	}

	std::cout << "]" << std::endl << std::endl;

	std::cout << "theor_utilisation = [" << std::endl;
	for (double directError = 0.0; directError < 1.0; directError += 0.01) {
		std::cout << (1 - directError) / (tau + 1);
		if (directError < 0.99)
			std::cout << ",";
		std::cout << std::endl;
	}
	std::cout << "]";
	//std::cout << "THEOR. UTILISATION: " << (1.0 - directError) / (1 + tau) << std::endl;
}