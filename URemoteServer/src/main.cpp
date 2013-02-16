#include "AI.h"

#include "Utils.h"
#include <Windows.h>
#include <iostream>
#include <sstream>

#include "ServerConfig.h"
#include "Translator.h"
#include "FileUtils.h"

using namespace std;

bool initProgram(unique_ptr<AIConfig>& aiConfig, unique_ptr<ServerConfig>& serverConfig);
bool initAiConfig(unique_ptr<AIConfig>& aiConfig, string& message);
bool initTranslator(Translator* translator, string& message);
bool initServerConfig(unique_ptr<ServerConfig>& serverConfig, string& message);

static const string LANGUAGE_DIR		= "lang\\";
static const string CONFIGURATION_DIR	= "conf\\";
static const string AI_CONF_FILE		= CONFIGURATION_DIR + "ai.conf";
static const string SERVER_CONF_FILE	= CONFIGURATION_DIR + "server.conf";

Logger* logger = Utils::getLogger();

int main()
{
	logger->debug("******************************************************");
	logger->debug("*****          Directory initialization          *****");
	logger->debug("******************************************************");
	
	CreateDirectoryA(LANGUAGE_DIR.c_str(), nullptr);
	if (ERROR_ALREADY_EXISTS == GetLastError()) {
		logger->debug("Language directory \"" + LANGUAGE_DIR + "\" already exists.");
	} else {
		logger->debug("Language directory \"" + LANGUAGE_DIR + "\" have been created.");
	}
	
	CreateDirectoryA(CONFIGURATION_DIR.c_str(), nullptr);
	if (ERROR_ALREADY_EXISTS == GetLastError()) {
		logger->debug("Configuration directory \"" + CONFIGURATION_DIR +  "\" already exists.");
	} else {
		logger->debug("Configuration directory \"" + CONFIGURATION_DIR + "\" have been created");
	}
	
	CreateDirectoryA(Logger::getLogDir().c_str(), nullptr);
	if (ERROR_ALREADY_EXISTS == GetLastError()) {
		logger->debug("Log directory \"" + Logger::getLogDir() + "\" already exists.");
	} else {
		logger->debug("Log directory \"" + Logger::getLogDir() + "\" have been created");
	}

	logger->setLogFile("URemote.log");

	logger->info("******************************************************");
	logger->info("*****               URemote Server               *****");
	logger->info("******************************************************");

	unique_ptr<AIConfig> aiConfig = nullptr;
	unique_ptr<ServerConfig> serverConfig = nullptr;
	if (!initProgram(aiConfig, serverConfig)) {
		return EXIT_FAILURE;
	}
	
	// Create the AI
	auto artificialIntelligence = unique_ptr<AI>(new AI(move(aiConfig)));
	
	// Start the server on the AI
	artificialIntelligence->StartConnection(move(serverConfig));
	
	Translator::freeInstance();
    return EXIT_SUCCESS;
}

/**
 * Initialize the AI, the translator and the Server
 */
bool initProgram(unique_ptr<AIConfig>& aiConfig, unique_ptr<ServerConfig>& serverConfig)
{
	logger->info("Program initialization...");

	bool programInitialized = false;
	string message = "";
	
	// Init config for the AI
	bool aiInitialized = initAiConfig(aiConfig, message);

	// Init translator
	auto translator = Translator::getInstance();
	bool translatorInitialized = initTranslator(translator, message);
	if (translatorInitialized && aiInitialized) {
		translator->setLanguage(aiConfig->Lang);
	}
	
	bool serverInitialized = initServerConfig(serverConfig, message);

	programInitialized = aiInitialized && translatorInitialized && serverInitialized;

	// Check program initializations
	if (programInitialized) {
		logger->info("Program initialized.");
	} else {
		logger->error("Program initialization failed !");
		MessageBoxA(nullptr, message.c_str(), nullptr, 0);
	}

	return programInitialized;
}

/**
 * Initialize the Translator.
 * Load the language files stored in LANGUAGE_DIR and add them to the translator.
 */
bool initTranslator(Translator* translator, string& message) 
{
	logger->info("Init Translator...");

	// TODO: Search files in directory LANGUAGE_DIR
	try {
		translator->addLanguage(Translator::LANG_EN, LANGUAGE_DIR + "\\en.lang");
	} catch (const exception& e) {
		logger->warning(e.what());
		message += e.what();
		message += "\n";
	}

	try {
		translator->addLanguage(Translator::LANG_FR, LANGUAGE_DIR + "\\fr.lang");
	} catch (const exception& e) {
		logger->warning(e.what());
		message += e.what();
		message += "\n";
	}
	/*
	auto files = FileUtils::list_files(LANGUAGE_DIR, false, ".*(\\.lang)$");
	for (auto file : files) {
		if (file.isDirectory()) {
			continue;
		}
		if (trunc_extension(file.getFilename()) == .) {
		}
		try {
			// TODO: Translator::LANG_XX dynamic
			translator->addLanguage(Translator::LANG_EN, file.getfullPath());
		} catch (const exception& e) {
			logger->warning(e.what());
			message += e.what();
			message += "\n";
		}
	}*/

	const auto isInitialized = translator->isInitialized();
	if (isInitialized) {
		logger->info("Translator OK.");
	} else {
		string errorMessage = "Translator KO : No language file available.";
		logger->error(errorMessage);
		message += errorMessage + "\n";
	}

	return isInitialized;
}

/**
 * Init config for the AI
 */
bool initAiConfig(unique_ptr<AIConfig>& aiConfig, string& message)
{
	logger->info("Init config for the AI...");
	bool aiInitialized = false;
	try {
		aiConfig = unique_ptr<AIConfig>(new AIConfig(AI_CONF_FILE));
		logger->info("AI config OK.");
		aiInitialized = true;
	} catch (const exception& e) {
		message += e.what();
		message += "\n";
		logger->error("AI config KO.");
	}
	return aiInitialized;
}

/**
 * Init config for the Server.
 * Load the ServerConfig object with the properties found in SERVER_CONF_FILE 
 */
bool initServerConfig(unique_ptr<ServerConfig>& serverConfig, string& message)
{
	// Init config for the server
	logger->info("Init config for the server...");
	bool serverInitialized = false;
	try {
		serverConfig = unique_ptr<ServerConfig>(new ServerConfig(SERVER_CONF_FILE));
		logger->debug("Server config OK.");
		serverInitialized = true;
	} catch (const exception& e) {
		logger->error(e.what());
		message += e.what();
		message += "\n";
		logger->error("Server config KO.");
	}
	return serverInitialized;
}