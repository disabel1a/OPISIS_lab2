#pragma once

#include <cstdlib>
#include <iostream>

class Queue {
private:
	struct Item {
		long message;
		Item* next;

		Item(long message) : message(message), next(nullptr) {};
	};

	Item* first;
	Item* last;

public:
	Queue() : first(nullptr), last(nullptr) {};

	~Queue() {
		while (first != nullptr) {
			Item* item = first;
			first = item->next;
			delete item;
		}
	};

	bool isEmpty() { return first == nullptr; };

	void pushBack(long message) {
		Item* item = new Item(message);
		if (isEmpty()) {
			first = item;
			last = item;
			return;
		}
		last->next = item;
		last = item;
	};

	long popFront() {
		if (isEmpty()) return NULL;
		long message = first->message;
		Item* item = first;
		first = item->next;
		delete item;
		return message;
	};

	long getFirst() {
		if (isEmpty()) return NULL;
		return first->message;
	};

	void removeFirst() {
		if (isEmpty()) return;
		Item* item = first;
		first = item->next;
		delete item;
	};

	void clear() {
		while (first != nullptr) {
			Item* item = first;
			first = item->next;
			delete item;
		}
	};

	void loadMessages(long messages[], unsigned int size) {
		for (size_t i = 0; i < size; ++i) {
			pushBack(messages[i]);
		}
	};
};

class TRDevice {
private:
	long polynome, mod;
	unsigned int deg, polynomeSize;

	unsigned int countSize(long val) {
		unsigned int counter = 0;
		while (val != 0) {
			val = val >> 1;
			counter++;
		}
		return counter;
	};

	long countRemainder(long& data) {
		long remainder = data;
		while (remainder >= mod) {
			remainder ^= (polynome << (countSize(remainder) - polynomeSize));
		}
		return remainder;
	}

public:
	TRDevice() {
		polynome = 0;
		deg = 0;
		polynomeSize = 0;
		mod = 0;
	};

	TRDevice(long polynome) : polynome(polynome) {
		deg = 0;
		polynomeSize = 0;
		mod = 1;
		while (polynome != 0) {
			if (polynome & 1)
				deg++;
			polynome = polynome >> 1;
			mod <<= 1;
			polynomeSize++;
		}
		mod >>= 1;
	};

	~TRDevice() {};

	unsigned int getDeg() { return deg; };

	long addControlSum(long message) {
		long sum = message << deg;
		sum = countRemainder(sum);
		return (message << deg) | sum;
	};

	bool checkSum(long& data) {
		if (countRemainder(data) == 0) {
			long message = data >> deg;
			if (data == addControlSum(message))
				return true;
		}
		return false;
	};

};

class Model {
private:
	double directError, returnError;
	TRDevice trd;
	unsigned int requestsNum, messageLength;
	double timer;
	long maxData;
	bool log;

	long lrand() {
		if (sizeof(int) < sizeof(long))
			return (static_cast<long>(rand()) << (sizeof(int) * 8)) | rand();
		return rand();
	};

	long sendData(long message) {
		if (log)
			std::cout << "MESSAGE: " << std::hex << message << std::endl << std::endl;

		timer++;
		return trd.addControlSum(message);
	};

	bool sendReceipt(long& data) {
		return trd.checkSum(data);
	};

	long dataError(double& tau) {
		timer += tau;
		if (directError == 0.0)
			return 0;

		double randValue = static_cast<double>(rand()) / RAND_MAX;
		if (randValue < directError) {
			return lrand() % maxData;
		}
		return 0;
	};

	bool receiptError(bool& receipt, double& tau) {
		timer += tau;
		if (returnError == 0.0)
			return receipt;

		double randValue = static_cast<double>(rand()) / RAND_MAX;
		if (randValue < returnError) {
			return receipt ^ 1;
		}
		return receipt;
	}

public:
	Model(long polynome, double directError, double returnError, unsigned int messageLength) : directError(directError), returnError(returnError), timer(0.0), requestsNum(0), messageLength(messageLength) {
		trd = TRDevice(polynome);
		srand(10);
		maxData = 1;
		for (unsigned int i = 0; i < (messageLength + trd.getDeg()); ++i) {
			maxData = maxData << 1;
		}
		log = true;
	};

	double runModel(long messages[], unsigned int size, unsigned int maxRequestsNum, double tau) {
		timer = 0;
		requestsNum = 0; // Number of requests in current queue
		unsigned int attempts = 0; // Local counter of same request

		unsigned int passedMessages = 0;
		tau = tau / 2;

		Queue queue;
		queue.loadMessages(messages, size);

		if (log)
			std::cout << "--------------- MODEL ----------------" << std::endl << std::endl;

		while (!queue.isEmpty()) {

			// Getting conrol sum (Transmitter work)
			long data = sendData(queue.getFirst());
			if (log)
				std::cout << "TIME(" << std::dec << timer  << ") SEND: " << std::hex << data << std::endl << std::endl;

			// Adding error to data (Direct channel work)
			long _data = data ^ dataError(tau);
			if (log)
				std::cout << "TIME(" << std::dec << timer << ") DIRECT CHANNEL: " << std::hex << _data << std::endl << std::endl;
			
			/*
			// Verify control sum (Receiver work)
			bool receipt = sendReceipt(_data);
			std::cout << "RECEIPT: " << receipt << std::endl << std::endl;
			*/

			// Verify control sum (Receiver work) (Stupid)
			bool receipt = (_data == data) ? true : false;
			if (log)
				std::cout << "RECEIPT: " << receipt << std::endl << std::endl;

			// Adding error to receipt (Return channel work)
			bool _receipt = receiptError(receipt, tau);
			if (log)
				std::cout << "TIME(" << std::dec << timer << ") RETURN CHANNEL: " << _receipt << std::endl << std::endl;

			// Increase counters
			attempts++;
			requestsNum++;

			// Check receipt
			if (_receipt) {
				passedMessages++;
				queue.removeFirst();
				attempts = 0;
				if (log) {
					std::cout << "--------------------------------------" << std::endl;
					std::cout << "--------------------------------------" << std::endl << std::endl;
				}
				continue;
			}

			// Check attempts border
			if (maxRequestsNum != 0 & attempts == maxRequestsNum) {
				queue.removeFirst();
				attempts = 0;
				if (log) {
					std::cout << "------------ SEND FAILED -------------" << std::endl;
					std::cout << "--------- TOO MANY ATTEMPTS ----------" << std::endl << std::endl;
				}
				continue;
			}
			if (log) {
				std::cout << "------------ SEND FAILED -------------" << std::endl << "------------ TRYING AGAIN ------------" << std::endl << std::endl;
			}
		}

		if (log) {
			std::cout << "TOTAL TIME: " << timer << std::endl;
			std::cout << "NUMBER OF REQUESTS: " << requestsNum << std::endl;
			std::cout << "MEAN REQUESTS: " << static_cast<double>(requestsNum) / size << std::endl;
			std::cout << "UTILISATION: " << static_cast<double>(requestsNum) / timer << std::endl;
			std::cout << std::endl;
		}

		return static_cast<double>(size) / timer;
	};

	void logOn() { log = true; };

	void logOff() { log = false; };

	void setDirectError(double directError) { this->directError = directError; };
};