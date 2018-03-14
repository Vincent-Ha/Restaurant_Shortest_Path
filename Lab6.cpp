#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <sstream>
#include <memory>
#include <cmath> 
#include <functional>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
using namespace std;

static const double RADIAN_CONVERSION_RATIO = 3.14159 / 180;
static const int RADIUS_OF_EARTH = 3959;

class Restaurant {
private:
	int number;
	string address;
	double latitude;
	double longitude;
	double distance;
public:
	Restaurant() : number(-1), address("N/A"), latitude(0), longitude(0), distance(0) {};
	void setNumber(int n);
	void setAddress(string a);
	void setLatitude(double la);
	void setlongitude(double lo);
	void setDistance(double d);
	int getNumber();
	string getAddress();
	double getLatitude();
	double getLongitude();
	double getDistance();
	bool operator== (const Restaurant& r);
	bool operator< (const Restaurant& r);
};

class Read_File {
private:
	ifstream fin;
	string data;
public:
	Read_File() {};
	void readFile(string file_name);
	string getData();
};

class XMLParser {
private:
	string data;
	vector <Restaurant> dataCollection;
	double toDouble(string s);
	void organizeQueueData(queue <string>& q);
	void grabAttributes(queue <string>& info, string line);
	function <double(XMLParser&, string)> to_num = &XMLParser::toDouble;
	function <void(XMLParser&, queue <string>&)> organize = &XMLParser::organizeQueueData;
	function <void(XMLParser&, queue <string>&, string)> parseTags = &XMLParser::grabAttributes;
public:
	XMLParser() : data("N/A") {};
	void setData(string d);
	void parse();
	vector <Restaurant> getDataCollection();
};

class RestaurantCollection {
private:
	unique_ptr <Read_File> reader;
	unique_ptr <XMLParser> parser;
	string rawData;
	vector <Restaurant> data;
	function <void(Read_File&, string)> read = &Read_File::readFile;
	function <void(XMLParser&, string)> dataSet = &XMLParser::setData;
	function <void(XMLParser&)> parse = &XMLParser::parse;
public:
	RestaurantCollection() : reader(new Read_File), parser(new XMLParser), rawData("N/A") {};
	void readFile(string fn);
	void parseFile();
	vector <Restaurant> getData();
};

class RestaurantGraph {
private:
	unique_ptr <RestaurantCollection> data_collector; 
	vector <Restaurant> restaurant_list;
	map <int, vector <Restaurant>> restaurant_adjacency_list;
	vector <Restaurant> path;
	double total_distance = 0;
	void addEdge(int index1, int index2);
	void haversine(int index, Restaurant& r);
	bool allRestaurantsPresent();
	function <void(RestaurantGraph&, int, Restaurant&)> haversine_calculator = &RestaurantGraph::haversine;
	function <bool(Restaurant&, const Restaurant&)> comparator = [](Restaurant& a, const Restaurant& b) {return a < b; };
public:
	RestaurantGraph() : data_collector(new RestaurantCollection) {};
	void readFile(string fn);
	void parseFile();
	void setUpRestaurantGraph();
	void findShortestPath(int start_point);
	void printShortestPath();
	double getTotalDistance();
	vector <Restaurant> getRestaurantList();
};

class UserInterface {
private:
	unique_ptr <RestaurantGraph> path_finder;
	vector <Restaurant> restaurants;
	function <void(RestaurantGraph&, string)> readData = &RestaurantGraph::readFile;
	function <void(RestaurantGraph&)> parseData = &RestaurantGraph::parseFile;
	function <void(RestaurantGraph&)> setUp = &RestaurantGraph::setUpRestaurantGraph;
	function <void(RestaurantGraph&, int)> findPath = &RestaurantGraph::findShortestPath;
public:
	UserInterface() : path_finder(new RestaurantGraph) {};
	void startProgram(string fn);
};

int main()
{
	UserInterface uI;
	uI.startProgram("c:/Users/Vincent Ha/Downloads/Restaurants.xml");
	return 0;
}

void Restaurant::setNumber(int n) {
	number = n;
}

void Restaurant::setAddress(string a) {
	address = a;
}

void Restaurant::setLatitude(double la) {
	latitude = la;
}

void Restaurant::setlongitude(double lo) {
	longitude = lo;
}

void Restaurant::setDistance(double d) {
	distance = d;
}

int Restaurant::getNumber() {
	return number;
}

string Restaurant::getAddress() {
	return address;
}

double Restaurant::getLatitude() {
	return latitude;
}

double Restaurant::getLongitude() {
	return longitude;
}

double Restaurant::getDistance() {
	return distance;
}

bool Restaurant::operator== (const Restaurant& r) {
	return address == r.address && latitude == r.latitude && longitude == r.longitude;
}

bool Restaurant::operator< (const Restaurant& r) {
	return distance < r.distance;
}

void Read_File::readFile(string file_name) {
	string name, content;
	string entireFile = "";
	fin.open(file_name);
	if (!fin.good()) {
		cerr << "Invalid File Path. Please Reload With Proper File Path." << endl;
		exit(-92);
	}

	while (!fin.eof()) {
		getline(fin, content);
		entireFile += content;
		entireFile += "\n";
		content.clear();
	}
	fin.close();
	data = entireFile;
}

string Read_File::getData() {
	return data;
}

double XMLParser::toDouble(string s) {
	double num;
	istringstream number_string(s);
	number_string >> num;
	return num;
}

void XMLParser::organizeQueueData(queue <string>& q) {
	Restaurant tempRestaurant;
	int index = 0;
	while (!q.empty()) {
		tempRestaurant.setAddress(q.front());
		q.pop();
		tempRestaurant.setLatitude(to_num(*this, q.front()));
		q.pop();
		tempRestaurant.setlongitude(to_num(*this, q.front()));
		q.pop();
		tempRestaurant.setNumber(index);
		index++;
		dataCollection.push_back(tempRestaurant);
	}
}

void XMLParser:: grabAttributes(queue <string>& info, string line) {
	regex inBetweenTags("[^<>]+");
	sregex_token_iterator collector(line.begin(), line.end(), inBetweenTags);
	collector++;
	info.push(static_cast<string>(*collector));
}

void XMLParser::setData(string d) {
	data = d;
}

void XMLParser::parse() {
	regex eachLine("<\\w+>.+|<\\w+>");
	regex tag("<\\w+>");
	regex endTag("\/(?!<)");
	stack <string> tags;
	queue <string> data_stack;
	string temp;

	data = regex_replace(data, endTag, "");
	sregex_token_iterator iter(data.begin(), data.end(), eachLine);
	tags.push(static_cast<string>(*iter));
	iter++;
	while (!tags.empty()) {
		temp = static_cast<string>(*iter);
		if (regex_match(temp, tag)) {
			if (tags.top() == temp)
				tags.pop();
			else
				tags.push(temp);
		}
		else
			parseTags(*this, data_stack, temp);
		iter++;
	}
	organize(*this, data_stack);
}

vector <Restaurant> XMLParser::getDataCollection() {
	return dataCollection;
}

void RestaurantCollection::readFile(string fn) {
	read(*reader, fn);
	rawData = reader->getData();
	dataSet(*parser, rawData);
}

void RestaurantCollection::parseFile() {
	parse(*parser);
	data = parser->getDataCollection();
}

vector <Restaurant> RestaurantCollection::getData() {
	return data;
}

void RestaurantGraph::addEdge(int index1, int index2) {
	if (index1 >= 0 && index1 < restaurant_list.size() && index2 >= 0 && index2 < restaurant_list.size())
		restaurant_adjacency_list[index1].push_back(restaurant_list[index2]);
	else {
		cerr << "Error. Improper Indexes" << endl;
		cerr << "Index1: " << index1 << "\tIndex2: " << index2 << endl;
		exit(index1);
	}
}

void RestaurantGraph::haversine(int index, Restaurant& r) {
	double difference_la = (restaurant_list[index].getLatitude() - r.getLatitude()) * RADIAN_CONVERSION_RATIO;
	double difference_lo = (restaurant_list[index].getLongitude() - r.getLongitude()) * RADIAN_CONVERSION_RATIO;
	double nA = pow(sin(difference_la / 2.), 2.) + cos(restaurant_list[index].getLatitude() * RADIAN_CONVERSION_RATIO) * pow(sin(difference_lo / 2.), 2.);
	double nB = 2.0 * atan2(sqrt(nA), sqrt(1. - nA));
	double distance = RADIUS_OF_EARTH * nB;
	r.setDistance(distance);
}

bool RestaurantGraph::allRestaurantsPresent() {
	vector <Restaurant>::iterator iter;
	for (auto& restaurant : restaurant_list) {
		iter = find(path.begin(), path.end(), restaurant);
		if (iter == path.end())
			return false;
	}
	return true;
}

void RestaurantGraph::readFile(string fn) {
	data_collector->readFile(fn);
}

void RestaurantGraph::parseFile() {
	data_collector->parseFile();
	restaurant_list = data_collector->getData();
}

void RestaurantGraph::setUpRestaurantGraph() {
	vector <vector <int>> edgeList = { { 3, 5, 7, 10, 12, 13, 14}, {2, 7, 11}, { 1, 5, 6, 8, 13}, { 0, 10, 12, 14}, { 7, 11, 12}, { 0, 2, 6, 13, 14}, { 2, 8, 13},  
	{ 0, 1, 3, 4, 11, 12}, { 2, 6, 9, 12, 13}, { 8, 13}, { 0, 3, 12, 13, 14}, { 1, 4, 7, 12, 13, 14},  { 0, 3, 7, 10, 14}, { 0, 2, 5, 6, 10, 14}, { 0, 3, 5, 10, 12, 13}, 
	{ 6, 8, 13, 16, 17, 18}, { 8, 9, 15, 17, 18}, { 8, 9, 15, 16, 18}, { 8, 9, 15, 16, 17 } };

	for (int vectorIndex = 0; vectorIndex < edgeList.size(); vectorIndex++) {
		for_each(edgeList[vectorIndex].begin(), edgeList[vectorIndex].end(), [&](int i) { addEdge(vectorIndex, i);});
		for_each(restaurant_adjacency_list[vectorIndex].begin(), restaurant_adjacency_list[vectorIndex].end(), [&](Restaurant& r) { haversine_calculator(*this, vectorIndex, r); });
		sort(restaurant_adjacency_list[vectorIndex].begin(), restaurant_adjacency_list[vectorIndex].end(), comparator);
	}
}

void RestaurantGraph::findShortestPath(int start_point) {
	map <int, vector <Restaurant>> graph_copy = restaurant_adjacency_list;
	Restaurant tempRestaurant = restaurant_list[start_point];
	int index = tempRestaurant.getNumber();
	path.push_back(tempRestaurant);
	while (!allRestaurantsPresent()) {
		if (graph_copy[index].size() == 0) {
			path.push_back(tempRestaurant);
			findShortestPath(index);
			break;
		}

		else {
			tempRestaurant = *min_element(graph_copy[index].begin(), graph_copy[index].end(), comparator);
			remove_if(graph_copy[index].begin(), graph_copy[index].end(), [&](const Restaurant& a) { return tempRestaurant == a; });
			index = tempRestaurant.getNumber();
			path.push_back(tempRestaurant);
			total_distance += tempRestaurant.getDistance();
		}
	}
}

void RestaurantGraph::printShortestPath() {
	int count = 0;
	for (auto& restaurant : path) {
		cout << "Restaurant #" << ++count << endl;
		cout << "--------------" << endl;
		cout << left << setw(12) << "Restaurant: " << restaurant.getNumber() << endl;
		cout << setw(12) << "Address: " << restaurant_list[restaurant.getNumber()].getAddress() << endl;
		cout << setw(12) << "Latitude: " << restaurant.getLatitude() << endl;
		cout << setw(11) << "Longitude: " << restaurant.getLongitude() << endl;
		cout << endl;
		if (count != path.size() && restaurant == path[count]) {
			cout << "No Further Path From Here. Restarting At This Point." << endl;
			cout << endl;
			count = 0;
		}
	}
}

double RestaurantGraph::getTotalDistance() {
	return total_distance;
}

vector <Restaurant> RestaurantGraph::getRestaurantList() {
	return restaurant_list;
}

void UserInterface::startProgram(string fn) {
	int startingPoint, endPoint;
	readData(*path_finder, fn);
	parseData(*path_finder);
	setUp(*path_finder);
	restaurants = path_finder->getRestaurantList();
	findPath(*path_finder, 18);
	cout << "Path to All the Restaurants" << endl;
	cout << "---------------------------" << endl;
	cout << endl;
	path_finder->printShortestPath();
	cout << "Total distance traveled: " << fixed << setprecision(3) << path_finder->getTotalDistance() << " miles" << endl;
}