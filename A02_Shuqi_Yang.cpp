/*
  DBS311 Assignment 2 
  Shuqi Yang
  08-01-2021
*/
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <occi.h>
#define MAX_PRODUCTS 5

using oracle::occi::Environment;
using oracle::occi::Connection;
using namespace oracle::occi;
using namespace std;

struct ShoppingCart {
	int product_id;
	double price;
	int quantity;
};

int mainMenu();
int customerLogin(Connection* conn, int customerID);
int addToCart(Connection* conn, struct ShoppingCart cart[]);
double findProduct(Connection* conn, int product_id);
void displayProducts(struct ShoppingCart cart[], int productCount);
int checkout(Connection* conn, struct ShoppingCart[], int customerId, int productCount);

int main() {
	Environment* env = nullptr;
	Connection* conn = nullptr;

	string user = "dbs311_212d05";
	string pass = "24838225";
	string constr = "myoracle12c.senecacollege.ca:1521/oracle12c";
	env = Environment::createEnvironment(Environment::DEFAULT);
	conn = env->createConnection(user, pass, constr);

	int cusID, prod_count;
	while (mainMenu()){
		cout << "Enter the customer ID:";
		cin >> cusID;
		if (!customerLogin(conn, cusID)) {
			cout << "The customer does not exist." << endl;
		}
		else {
			ShoppingCart cart[MAX_PRODUCTS];
			prod_count = addToCart(conn, cart);
			displayProducts(cart, prod_count);
			checkout(conn, cart, cusID, prod_count);
		}
	}
	env->terminateConnection(conn);
	Environment::terminateEnvironment(env);
	cout << "Good Bye!..." << endl;
	return 0;
}

int mainMenu() {
	int userInput;
	char NL;
	do {
		cout << "******************** Main Menu ********************" << endl;
		cout << "1) Login" << endl << "0) Exit" << endl << "Enter an option (0-1):";
		while (!(cin >> userInput)) {
			cin.clear();
			cin.ignore(INT_MAX, '\n');
			cout << "You entered a wrong value. Enter an option (0-1):";
		};
		cin.get(NL);
		if (NL != '\n') {
			cin.clear();
			cin.ignore(INT_MAX, '\n');
		};
	} while (userInput < 0 || userInput > 1 || NL != '\n');
	return userInput;
}


int customerLogin(Connection* conn, int customerID) {
	int exist{};
	string pro_call = "BEGIN find_customer(:1, :2); END;";
	try {
		Statement* stmt = nullptr;
		stmt = conn->createStatement(pro_call);
		stmt->setNumber(1, customerID);
		stmt->registerOutParam(2, OCCINUMBER, exist);
		stmt->execute();
		exist = stmt->getNumber(2);
		conn->terminateStatement(stmt);	
		return exist;
	}catch (SQLException& sqlExcp) {
		cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
	}

}
int addToCart(Connection* conn, struct ShoppingCart cart[]) {
	cout << "-------------- Add Products to Cart --------------" << endl;
	int count{}, quantity{};
	int proID_input, more;
	char NL;
	do {
		cout << "Enter the product ID:";
		while (!(cin >> proID_input)) {
			cin.clear();
			cin.ignore(INT_MAX, '\n');
			cout << "You entered a wrong value. Please enter a valid Product ID:";
		};
		cin.get(NL);
		if (NL != '\n') {
			cin.clear();
			cin.ignore(INT_MAX, '\n');
		};
		double price = findProduct(conn, proID_input);
		if (!price) {
			cout << "The product does not exists.Try again..." << endl;
		}
		else {
			cout << "Product Price:" << price << endl;
			cout << "Enter the product Quantity:";
			while (!(cin >> quantity)) {
				cin.clear();
				cin.ignore(INT_MAX, '\n');
				cout << "You entered a wrong value. Please enter a valid Quantity:";
			};
			cin.get(NL);
			if (NL != '\n') {
				cin.clear();
				cin.ignore(INT_MAX, '\n');
			};
			cart[count].product_id = proID_input;
			cart[count].price = price;
			cart[count].quantity = quantity;
			count++;
		}
		if (count < MAX_PRODUCTS) {
			cout << "Enter 1 to add more products or 0 to checkout:";
			while (!(cin >> more)) {
				cin.clear();
				cin.ignore(INT_MAX, '\n');
				cout << "You entered a wrong value. Please enter (0-1):";
			};
			cin.get(NL);
			if (NL != '\n') {
				cin.clear();
				cin.ignore(INT_MAX, '\n');
			};
		}
	} while (count < MAX_PRODUCTS && more != 0);
	return count;
}

double findProduct(Connection* conn, int product_id) {
	double price{};
	string pro_call = "BEGIN find_product(:1, :2); END;";
	try {
		Statement* stmt = nullptr;
		stmt = conn->createStatement(pro_call);
		stmt->setNumber(1, product_id);
		stmt->registerOutParam(2, OCCIDOUBLE, price);
		//stmt->executeUpdate();
		stmt->execute();
		price = stmt->getDouble(2);
		conn->terminateStatement(stmt);
		return price;
	}
	catch (SQLException& sqlExcp) {
		cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
	};
}

void displayProducts(struct ShoppingCart cart[], int productCount) {
	double total{};
	cout << "------- Ordered Products ---------" << endl;
	for (int i = 0; i < productCount; i++) {
		cout << "---Item " << i + 1 << endl;
		cout << "Product ID:" << cart[i].product_id << endl;
		cout << "Price:" << cart[i].price << endl;
		cout << "Quantity:" << cart[i].quantity << endl;
		total += cart[i].price * cart[i].quantity;
	}
	cout << "----------------------------------" << endl << "Total:" << total << endl;
}

int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount) {
	char input, NL;
	do {
		cout << "Would you like to checkout? (Y/y or N/n)";
		while (!(cin >> input)) {
			cin.clear();
			cin.ignore(INT_MAX, '\n');
			cout << "Wrong input.Try again..." << endl;
		};
		cin.get(NL);
		if (NL != '\n') {
			cin.clear();
			cin.ignore(INT_MAX, '\n');
		};
		if (input != 'Y' && input != 'y' && input != 'N' && input != 'n')
			cout << "Wrong input.Try again..." << endl;
	} while (input != 'Y' && input != 'y' && input != 'N' && input != 'n');
	
	if (input == 'N' || input == 'n') {
		cout << "The order is cancelled." << endl;
		return 0;
	}
	else {
		int order_id{};
		string pro_call = "BEGIN add_order(:1, :2); END;";
		try {
			Statement* stmt = nullptr;
			stmt = conn->createStatement(pro_call);
			stmt->setNumber(1, customerId);
			stmt->registerOutParam(2, OCCINUMBER, order_id);
			//stmt->executeUpdate();
			stmt->execute();
			order_id = stmt->getNumber(2);
			conn->terminateStatement(stmt);
		}
		catch (SQLException& sqlExcp) {
			cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
		};

		for (int i = 0; i < productCount; i++) {
			string pro_call = "BEGIN add_order_item(:1, :2, :3, :4, :5); END;";
			try {
				Statement* stmt = nullptr;
				stmt = conn->createStatement(pro_call);
				stmt->setNumber(1, order_id);
				stmt->setNumber(2, i + 1);
				stmt->setNumber(3, cart[i].product_id);
				stmt->setNumber(4, cart[i].quantity);
				stmt->setNumber(5, cart[i].price);
				stmt->executeUpdate();
				conn->commit();
				conn->terminateStatement(stmt);
			}
			catch (SQLException& sqlExcp) {
				cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
			};
		}
		cout << "The order is successfully completed." << endl;
		return 1;
	}
}