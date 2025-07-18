#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <sstream>
using namespace std;
namespace fs = std::filesystem;
// MiniGit commit hash function
// Created and maintained by Ruhama Beshada,sifen fikru and kalkidan asdesach
// This function generates a simple deterministic hash to identify commits based
// Generate a simple hash from a string (used for file content add commits)
string simpleHash(const string &content)
{
    unsigned long hash = 5381;
    for (char c : content)
    {
        hash = ((hash << 5) + hash) + c;
    }
    return to_string(hash);
}

// Initialize a new MiniGit repository with required folders and files
void initMiniGit()
{
    if (!fs::exists(".minigit"))
    {
        if (!fs::create_directory(".minigit"))
        {
            cout << "Error: Failed to create .minigit directory." << endl;
            return;
        }
    }
    else
    {
        cout << "Note: .minigit directory already exists." << endl;
    }
    if (!fs::exists(".minigit/objects"))
    {
        if (!fs::create_directory(".minigit/objects"))
        {
            cout << "Error: Failed to create .minigit/objects directory." << endl;
            return;
        }
        else
        {
            cout << "Note: .minigit/objects directory already exists." << endl;
        }
    }
    if (!fs::exists(".minigit/refs"))
    {
        if (!fs::create_directory(".minigit/refs"))
        {
            cout << "Error: Failed to create .minigit/refs directory." << endl;
            return;
        }
        else
        {
            cout << "Note: .minigit/refs directory already exists." << endl;
        }
    }
    if (!fs::exists(".minigit/refs/heads"))
    {
        if (!fs::create_directory(".minigit/refs/heads"))
        {
            cout << "Error: Failed to create .minigit/refs/heads directory." << endl;
            system("mkdir .minigit\\refs\\heads");
            if (!fs::exists(".minigit/refs/heads"))
            {
                cout << "Error: Fallback creation of .minigit/refs/heads failed." << endl;
                return;
            }
        }
    }
    else
    {
        cout << "Note: .minigit/refs/heads directory already exists." << endl;
    }

    ofstream headFile(".minigit/HEAD");
    if (!headFile.is_open())
    {
        cout << "Error: Failed to create .minigit/HEAD file." << endl;
        return;
    }
    headFile << "refs/heads/main";
    headFile.close();
    ofstream mainBranch(".minigit/refs/heads/main");
    if (!mainBranch.is_open())
    {
        cout << "Error: Failed to create .minigit/refs/heads/main file." << endl;
        return;
    }
    mainBranch.close();
    cout << "Initialized empty MiniGit repository in .minigit/" << endl;
}
// stage a file by hashing its content and storing it in objects
void addFile(const string &filename)
{
    cout << "Attempting to add file: " << filename << endl;
    ifstream file(filename);
    if (!file.is_open())
    {
        cout << "Error: File " << filename << " not found." << endl;
        return;
    }
    cout << "File opened successfully. " << endl;
    stringstream buffer;
    buffer << file.rdbuf();
    string content = buffer.str();
    file.close();
    cout << "File content read: " << content.length() << " bytes." << endl;
    string hash = simpleHash(content);
    cout << "computed hash: " << hash << endl;
    string objectPath = ".minigit/objects/" + hash;
    if (!fs::exists(objectPath))
    {
        ofstream blobFile(objectPath);
        if (!blobFile.is_open())
        {
            cout << "Error: Failed to create object file for" << filename << endl;
            return;
        }
        blobFile << content;
        blobFile.close();
        cout << "Stored content in " << objectPath << endl;
    }
    else
    {
        cout << "Object " << objectPath << "already exists." << endl;
    }
    bool alreadyInIndex = false;
    if (fs::exists(".minigit/index"))
    {
        ifstream indexRead(".minigit/index");
        string line;
        while (getline(indexRead, line))
        {
            stringstream ss(line);
            string indexedFile, indexedHash;
            ss >> indexedFile >> indexedHash;
            if (indexedFile == filename && indexedHash == hash)
            {
                alreadyInIndex = true;
                break;
            }
        }
        indexRead.close();
    }
    if (!alreadyInIndex)
    {
        ofstream indexFile(".minigit/index", ios::app);
        if (!indexFile.is_open())
        {
            cout << "Error: Failed to create .minigit/index file." << endl;
            return;
        }
        indexFile << filename << " " << hash << endl;
        indexFile.close();
        cout << "Added entry to.minigit/index." << endl;
    }
    else
    {
        cout << "File" << filename << " already in staging area with hash" << hash << "." << endl;
    }
    cout << "Added" << filename << " to staging area." << endl;
}

// create a commit from staged files, store in objects, and update branch reference
void commit(const string &message)
{
    if (!fs::exists(".minigit/index"))
    {
        cout << " Error: Nothing to commit, staging area is empt. " << endl;
        return;
    }
    stringstream commitContent;
    ifstream indexFile(".minigit/index");
    if (!indexFile.is_open())
    {
        cout << "Error: Failed to open .minigit/index." << endl;
        return;
    }
    string line;
    while (getline(indexFile, line))
    {
        commitContent << line << "\n";
    }
    indexFile.close();
    commitContent << "message: " << message << "\n";
    string commitData = commitContent.str();
    string commitHash = simpleHash(commitData);
    string commitPath = ".minigit/objects/" + commitHash;
    ofstream commitFile(commitPath);
    if (!commitFile.is_open())
    {
        cout << "Error: Failed to create commit object at " << commitPath << endl;
        return;
    }
    commitFile << commitData;
    commitFile.close();
    ofstream mainBranch(".minigit/refs/heads/main");
    if (!mainBranch.is_open())
    {
        cout << "Error: Failed to update .minigit/refs/heads/main." << endl;
        return;
    }
    mainBranch << commitHash;
    mainBranch.close();
    fs::remove(".minigit/index");
    cout << "Committed with hash" << commitHash << ": " << message << endl;
}

// show the latest commit and its message
void log()
{
    if (!fs::exists(".minigit/refs/heads/main"))
    {
        cout << "Error: No commits found." << endl;
        return;
    }
    ifstream mainBranch(".minigit/refs/heads/main");
    if (!mainBranch.is_open())
    {
        cout << "Error: Failed to open .minigit/refs/heads/main." << endl;
        return;
    }
    string commitHash;
    getline(mainBranch, commitHash);
    mainBranch.close();
    if (commitHash.empty())
    {
        cout << "Error: No commits found." << endl;
        return;
    }
    string commitPath = ".minigit/objects/" + commitHash;
    ifstream commitFile(commitPath);
    if (!commitFile.is_open())
    {
        cout << "Error: Failed to open commit object at " << commitPath << endl;
        return;
    }
    string line;
    string message;
    bool inMessage = false;
    while (getline(commitFile, line))
    {
        if (line.rfind("message: ", 0) == 0)
        {
            message = line.substr(9);
            inMessage = true;
        }
        else if (inMessage)
        {
            message += "\n" + line;
        }
    }
    commitFile.close();
    cout << "commit " << commitHash << endl;
    cout << "   " << message << endl;
}

// ceate a new branch pointing to the lastest commit
void branch(const string &branchName)
{
    // Check if branch already exists
    string branchPath = ".minigit/refs/heads/" + branchName;
    if (fs::exists(branchPath))
    {
        cout << "Error: 'Branch" << branchName << " already exists." << endl;
        return;
    }
    // Get current commit hash from HEAD (refs/heads/main)
    if (!fs::exists(".minigit/refs/heads/main"))
    {
        cout << "Error: No commits found. Create a commit before branching." << endl;
        return;
    }

    ifstream mainBranch(".minigit/refs/heads/main");
    if (!mainBranch.is_open())
    {
        cout << "Error: Failed to open minigit/refs/heads/main." << endl;
        return;
    }
    string currentCommitHash;
    getline(mainBranch, currentCommitHash);
    mainBranch.close();

    if (currentCommitHash.empty())
    {
        cout << "Error: No commits found. Create a commit before 'branching." << endl;
        return;
    }
    // Create new branch file with current commit hash CRLF
    ofstream branchFile(branchPath);
    if (!branchFile.is_open())
    {
        cout << "Error: Failed to create branch." << branchName << "." << endl;
        return;
    }
    branchFile << currentCommitHash;
    branchFile.close();

    cout << "Created branch " << branchName << "." << endl;
}

// Switches to the specified branch by updating the HEAD reference
void checkout(const std::string &branchName)
{
    string branchPath = ".minigit/refs/heads/" + branchName;
    if (!fs::exists(branchPath))
    {
        cout << "Error:Branch" << branchName << " does not exist." << endl;
        return;
    }
    ofstream headFile(".minigit/HEAD");
    if (!headFile.is_open())
    {
        cout << "Error:Failed to update .minigit/HEAD." << endl;
        return;
    }
    headFile << "refs/heads/" << branchName;
    headFile.close();
    cout << "Switched to branch" << branchName << "." << endl;
}

// Shows line-by-line differences between a file and its staged version
void diff(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cout << "Error: File " << filename << " not found." << endl;
        return;
    }
    stringstream buffer;
    buffer << file.rdbuf();
    string currentContent = buffer.str();
    file.close();

    // if it doesn't exists
    if (!fs::exists(".minigit/index"))
    {
        cout << "No staged changes for " << filename << "." << endl;
        return;
    }
    ifstream indexFile(".minigit/index");
    string line;
    string stagedHash;
    while (getline(indexFile, line))
    {
        stringstream ss(line);
        string indexedFile, hash;
        ss >> indexedFile >> hash;
        if (indexedFile == filename)
        {
            stagedHash = hash;
            break;
        }
    }
    indexFile.close();
    if (stagedHash.empty())
    {
        cout << "File " << filename << " not staged. " << endl;
        return;
    }
    string stagedPath = ".minigit/objects/" + stagedHash;
    ifstream stagedFile(stagedPath);
    if (!stagedFile.is_open())
    {
        cout << "Error: Staged version of " << filename << " not found." << endl;
        return;
    }
    buffer.str("");
    buffer << stagedFile.rdbuf();
    string stagedContent = buffer.str();
    stagedFile.close();

    istringstream currentStream(currentContent);
    istringstream stagedStream(stagedContent);
    string currentLine, stagedLine;
    
    bool differ = false;
    while (getline(currentStream, currentLine) || getline(stagedStream, stagedLine))
    {
        if (currentLine != stagedLine)
        {
            if (currentLine.empty())
            {
                cout << "-" << stagedLine << endl;
            }
            else if (stagedLine.empty())
            {
                cout << "+" << currentLine << endl;
            }
            else
            {
                cout << "-" << stagedLine << endl;
                cout << "+" << currentLine << endl;
            }
            differ = true;
        }
        else if (!currentLine.empty())
        {
            cout << "  " << currentLine << endl;
        }
    }
    if (!differ)
    {
        cout << "No differences found between " << filename << " and its staged version." << endl;
    }
}
// Merges the source branch into the target branch by updating its commit hash.
void merge(const string &sourceBranch, const string &targetBranch)
{
    string sourcePath = ".minigit/refs/heads/" + sourceBranch;
    string targetPath = ".minigit/refs/heads/" + targetBranch;

    // Check if source branch exists
    if (!fs::exists(sourcePath))
    {
        cout << "Error: Source branch " << sourceBranch << " does not exist." << endl;
        return;
    }

    // Check if target branch exists
    if (!fs::exists(targetPath))
    {
        cout << "Error: Target branch " << targetBranch << " does not exist." << endl;
        return;
    }

    // Read source branch's commit hash
    ifstream sourceFile(sourcePath);
    string sourceCommit;
    getline(sourceFile, sourceCommit);
    sourceFile.close();

    // Update target branch with source commit hash
    ofstream targetFile(targetPath);
    if (!targetFile.is_open())
    {
        cout << "Error: Failed to update " << targetPath << "." << endl;
        return;
    }
    targetFile << sourceCommit;
    targetFile.close();

    // Update HEAD if target branch is currently checked out
    ifstream headFile(".minigit/HEAD");
    string currentBranch;
    getline(headFile, currentBranch);
    headFile.close();
    if (currentBranch == "refs/heads/" + targetBranch)
    {
        ofstream headUpdate(".minigit/HEAD");
        if (!headUpdate.is_open())
        {
            cout << "Error: Failed to update .minigit/HEAD." << endl;
            return;
        }
        headUpdate << currentBranch;
        headUpdate.close();
    }

    cout << "Merged " << sourceBranch << " into " << targetBranch << "." << endl;
}

// main should be re optimized
// Parses and executes MiniGit commands from command-line arguments.
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: minigit <command>" << endl;
        return 1;
    }
    string command = argv[1];
    if (command == "init")
    {
        initMiniGit();
    }
    else if (command == "add" && argc == 3)
    {
        addFile(argv[2]);
    }
    else if (command == "commit" && argc == 3)
    {
        commit(argv[2]);
    }
    else if (command == "log" && argc == 2)
    {
        log();
    }
    else if (command == "checkout" && argc == 3)
    {
        checkout(argv[2]);
    }
    else if (command == "branch" && argc == 3)
    {
        branch(argv[2]);
    }
    else if (command == "diff" && argc == 3)
    {
        diff(argv[2]);
    }
    else if (command == "merge" && argc == 5 && string(argv[2]) == "-b")
    {
        merge(argv[4], argv[3]);
    }
    else
    {
        cout << "Unknown command: " << command << endl;
        return 1;
    }
    return 0;
}