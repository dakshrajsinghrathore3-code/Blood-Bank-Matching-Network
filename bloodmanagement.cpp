#include <iostream> 
#include <string> 
#include <vector> 
#include <fstream> 
using namespace std; 
 
const int TYPES = 8; 
string bloodNames[TYPES] = {"O-", "O+", "A-", "A+", "B-", "B+", "AB-", "AB+"}; 
 
int findBloodTypeIndex(string type) { 
    for (int i = 0; i < TYPES; i++) { 
        if (bloodNames[i] == type) return i; 
    } 
    return -1; 
} 
 
class BloodBank { 
private: 
    string name; 
    int stock[TYPES]; 
 
public: 
    BloodBank(string bankName, int initialStock[TYPES]) { 
        name = bankName; 
        for (int i = 0; i < TYPES; i++) { 
            stock[i] = initialStock[i]; 
        } 
    } 
 
    string getName() { 
        return name; 
    } 
 
    int getUnits(int typeIndex) { 
        return stock[typeIndex]; 
    } 
 
    void addUnits(int typeIndex, int units) { 
        stock[typeIndex] = stock[typeIndex] + units; 
    } 
 
    int removeUnits(int typeIndex, int units) { 
        int taken = units; 
        if (stock[typeIndex] < units) { 
            taken = stock[typeIndex]; 
        } 
        stock[typeIndex] = stock[typeIndex] - taken; 
        return taken; 
    } 
 
    void printStock(int index) { 
        cout << "  [" << index << "] " << name << ":\n      "; 
        for (int i = 0; i < TYPES; i++) { 
            cout << bloodNames[i] << "=" << stock[i] << "  "; 
        } 
        cout << "\n"; 
    } 
}; 
 
class Request { 
private: 
    string bloodType; 
    int unitsNeeded; 
    int urgency;   // 1 = Routine, 2 = Urgent, 3 = Critical 
 
public: 
    Request(string type, int units, int urgencyLevel) { 
        bloodType = type; 
        unitsNeeded = units; 
 
        // Anything that is not 2 or 3 is treated as Routine. 
        if (urgencyLevel == 2 || urgencyLevel == 3) { 
            urgency = urgencyLevel; 
        } else { 
            urgency = 1; 
        } 
    } 
 
    string getType() { 
        return bloodType; 
    } 
 
    int getUnits() { 
        return unitsNeeded; 
    } 
 
    int getPriority() { 
        return urgency; 
    } 
 
    string getLabel() { 
        if (urgency == 3) return "Critical"; 
        if (urgency == 2) return "Urgent"; 
        return "Routine"; 
    } 
}; 
 
class BloodNetwork { 
private: 
    vector<BloodBank> banks; 
 
public: 
    void addBank(BloodBank bank) { 
        banks.push_back(bank); 
    } 
 
    int bankCount() { 
        return (int)banks.size(); 
    } 
 
    string bankName(int i) { 
        return banks[i].getName(); 
    } 
 
    void printAllBanks() { 
        cout << "\n--- Current Stock (choose a bank by its number) ---\n"; 
        for (int i = 0; i < bankCount(); i++) { 
            banks[i].printStock(i); 
        } 
    } 
 
    void donate(int bankIndex, int typeIndex, int units) { 
        banks[bankIndex].addUnits(typeIndex, units); 
    } 
 
    bool fulfill(int requestingBankIndex, Request req) { 
        int type = findBloodTypeIndex(req.getType()); 
        if (type == -1) { 
            cout << "Unknown blood type: " << req.getType() << "\n"; 
            return false; 
        } 
 
        int needed = req.getUnits(); 
 
        cout << "\n[" << req.getLabel() << "] " 
             << banks[requestingBankIndex].getName() 
             << " needs " << needed << " unit(s) of " << req.getType() << "\n"; 
 
        // Step 1: use the requesting bank's own stock first. 
        int used = banks[requestingBankIndex].removeUnits(type, needed); 
        needed = needed - used; 
        if (used > 0) { 
            cout << "  Used " << used << " unit(s) from own stock.\n"; 
        } 
 
        for (int i = 0; i < bankCount(); i++) { 
            if (needed == 0) break; 
            if (i == requestingBankIndex) continue; 
 
            int taken = banks[i].removeUnits(type, needed); 
            if (taken > 0) { 
                banks[requestingBankIndex].addUnits(type, taken); 
                needed = needed - taken; 
                cout << "  Transferred " << taken << " unit(s) from " 
                     << banks[i].getName() << " -> " 
                     << banks[requestingBankIndex].getName() << "\n"; 
            } 
        } 
 
        int oNegIndex = 0; 
        if (needed > 0 && type != oNegIndex) { 
            for (int i = 0; i < bankCount(); i++) { 
                if (needed == 0) break; 
                if (i == requestingBankIndex) continue; 
 
                int taken = banks[i].removeUnits(oNegIndex, needed); 
                if (taken > 0) { 
                    banks[requestingBankIndex].addUnits(type, taken); 
                    needed = needed - taken; 
                    cout << "  Substituted " << taken << " unit(s) of O- from " 
                         << banks[i].getName() << " -> " 
                         << banks[requestingBankIndex].getName() << "\n"; 
                } 
            } 
        } 
 
        cout << "----------------------------------------\n"; 
        if (needed == 0) { 
            cout << "  Your request has been successfully submitted!\n"; 
            cout << "  Blood is on the way to " 
                 << banks[requestingBankIndex].getName() << ".\n"; 
            return true; 
        } else { 
            cout << "  Sorry -- we could only arrange part of this request.\n"; 
            cout << "  Still short by " << needed << " unit(s) of " 
                 << req.getType() << " across the whole network.\n"; 
            return false; 
        } 
    } 
}; 
 
int askBloodType() { 
    string type; 
    int index = -1; 
    while (index == -1) { 
        cout << "Blood type (e.g. O-, A+, AB-): "; 
        cin >> type; 
        index = findBloodTypeIndex(type); 
        if (index == -1) { 
            cout << "  That's not a valid blood type. Valid options: "; 
            for (int i = 0; i < TYPES; i++) cout << bloodNames[i] << " "; 
            cout << "\n"; 
        } 
    } 
    return index; 
} 
 
void saveRequestToFile(int bankIndex, string bankName, Request req, bool fulfilled) { 
    ofstream file("blood_requests.txt", ios::app); 
 
    if (!file) { 
        cout << "Could not open request file.\n"; 
        return; 
    } 
 
    file << "Hospital: " << bankName << "\n"; 
    file << "Blood Type: " << req.getType() << "\n"; 
    file << "Units Requested: " << req.getUnits() << "\n"; 
    file << "Urgency: " << req.getLabel() << "\n"; 
    file << "Status: " << (fulfilled ? "Fulfilled" : "Partially Fulfilled") << "\n"; 
    file << "----------------------------------------\n"; 
 
    file.close(); 
    cout << "Request saved to blood_requests.txt\n"; 
} 
 
int main() { 
    BloodNetwork network; 
 
    int stockA[TYPES] = {2, 10, 2, 8, 1, 6, 1, 4}; 
    int stockB[TYPES] = {15, 20, 10, 12, 8, 10, 5, 6}; 
    int stockC[TYPES] = {1, 3, 1, 2, 0, 2, 0, 1}; 
    int stockD[TYPES] = {1, 3, 9, 8, 8, 5, 2, 12}; 
 
    network.addBank(BloodBank("City General Hospital", stockA)); 
    network.addBank(BloodBank("Riverside Blood Bank", stockB)); 
    network.addBank(BloodBank("St. Mary's Medical Center", stockC)); 
    network.addBank(BloodBank("Downtown Clinic", stockD)); 
 
    int choice = 0; 
    do { 
        cout << "\n===== Blood Bank Matching Network =====\n"; 
        cout << "1. View all bank stock\n"; 
        cout << "2. Request blood for a bank\n"; 
        cout << "3. Add donated stock to a bank\n"; 
        cout << "4. Exit\n"; 
        cout << "Enter choice: "; 
        cin >> choice; 
 
        if (choice == 1) { 
            network.printAllBanks(); 
        } 
        else if (choice == 2) { 
            network.printAllBanks(); 
            cout << "\nWhich hospital needs blood? (enter its number 0-" 
                 << network.bankCount() - 1 << "): "; 
            int b; 
            cin >> b; 
            if (b < 0 || b >= network.bankCount()) { 
                cout << "Invalid bank.\n"; 
                continue; 
            } 
 
            int typeIndex = askBloodType(); 
 
            cout << "Units needed: "; 
            int units; 
            cin >> units; 
 
            cout << "Urgency (1=Routine, 2=Urgent, 3=Critical): "; 
            int urgency; 
            cin >> urgency; 
 
            Request req(bloodNames[typeIndex], units, urgency); 
            bool fulfilled = network.fulfill(b, req); 
 
            // Save this request in a text file. 
            saveRequestToFile(b, network.bankName(b), req, fulfilled); 
        } 
        else if (choice == 3) { 
            network.printAllBanks(); 
            cout << "\nWhich hospital received the donation? (enter its number 0-" 
                 << network.bankCount() - 1 << "): "; 
            int b; 
            cin >> b; 
            if (b < 0 || b >= network.bankCount()) { 
                cout << "Invalid bank.\n"; 
                continue; 
            } 
 
            int typeIndex = askBloodType(); 
 
            cout << "Units donated: "; 
            int units; 
            cin >> units; 
 
            network.donate(b, typeIndex, units); 
            cout << "Added " << units << " unit(s) of " << bloodNames[typeIndex] 
                 << " to " << network.bankName(b) << "\n"; 
        } 
        else if (choice != 4) { 
            cout << "Invalid choice.\n"; 
        } 
 
    } while (choice != 4); 
 
    cout << "Goodbye!\n"; 
    return 0; 
} 