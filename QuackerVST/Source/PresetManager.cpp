/*
  ==============================================================================

    PresetManager.cpp
    Created: 21 Feb 2025 7:34:15pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "PresetManager.h"

// Static helper functions for Preset struct
juce::String PresetManager::Preset::sanitizeName(const juce::String& name) {
    juce::String sanitized = name.trim();
    
    // Remove invalid characters
    sanitized = sanitized.removeCharacters("\\/:*?\"<>|");
    
    // Limit length
    if (sanitized.length() > MAX_PRESET_NAME_LENGTH) {
        sanitized = sanitized.substring(0, MAX_PRESET_NAME_LENGTH);
    }
    
    // Ensure not empty
    if (sanitized.isEmpty()) {
        sanitized = "Untitled";
    }
    
    return sanitized;
}

juce::String PresetManager::Preset::sanitizeCategory(const juce::String& category) {
    juce::String sanitized = category.trim();
    
    // Remove invalid path characters but keep forward slashes for hierarchy
    sanitized = sanitized.removeCharacters("\\:*?\"<>|");
    
    // Limit length
    if (sanitized.length() > MAX_CATEGORY_LENGTH) {
        sanitized = sanitized.substring(0, MAX_CATEGORY_LENGTH);
    }
    
    // Default to User if empty
    if (sanitized.isEmpty()) {
        sanitized = "User";
    }
    
    return sanitized;
}

bool PresetManager::Preset::isValid() const noexcept {
    return name.isNotEmpty() &&
           category.isNotEmpty() &&
           state.isValid() &&
           state.hasType(state.getType());
}

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts)
    : apvts(apvts)
{
    // Set up preset directory with platform-specific handling
    try {
        #if JUCE_MAC
        juce::File appSupportDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
        juce::File companyDir = appSupportDir.getChildFile("DeividsHvostovsDSP");
        juce::File pluginDir = companyDir.getChildFile("TremoloViola");
        presetDirectory = pluginDir.getChildFile("Presets");
        #elif JUCE_WINDOWS
        juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
        juce::File companyDir = appDataDir.getChildFile("DeividsHvostovsDSP");
        juce::File pluginDir = companyDir.getChildFile("TremoloViola");
        presetDirectory = pluginDir.getChildFile("Presets");
        #else
        juce::File homeDir = juce::File::getSpecialLocation(juce::File::userHomeDirectory);
        juce::File companyDir = homeDir.getChildFile(".DeividsHvostovsDSP");
        juce::File pluginDir = companyDir.getChildFile("TremoloViola");
        presetDirectory = pluginDir.getChildFile("Presets");
        #endif

        if (!createPresetDirectory()) {
            reportError(ErrorCode::DirectoryCreationFailed,
                       "Failed to create preset directory at: " + presetDirectory.getFullPathName());
        }
        
        scanForPresets();
    }
    catch (const std::exception& e) {
        reportError(ErrorCode::DirectoryCreationFailed,
                   "Exception during initialization: " + juce::String(e.what()));
    }
}

PresetManager::~PresetManager()
{
    // Cleanup is automatic with smart pointers
}

bool PresetManager::createPresetDirectory()
{
    try {
        // Create the entire directory hierarchy with error checking
        juce::File companyDir = presetDirectory.getParentDirectory().getParentDirectory();
        
        if (!companyDir.exists() && !companyDir.createDirectory()) {
            DBG("Failed to create company directory at: " + companyDir.getFullPathName());
            return false;
        }
        
        juce::File pluginDir = presetDirectory.getParentDirectory();
        if (!pluginDir.exists() && !pluginDir.createDirectory()) {
            DBG("Failed to create plugin directory at: " + pluginDir.getFullPathName());
            return false;
        }
        
        if (!presetDirectory.exists() && !presetDirectory.createDirectory()) {
            DBG("Failed to create preset directory at: " + presetDirectory.getFullPathName());
            return false;
        }
        
        // Create standard subdirectories
        juce::File factoryDir = presetDirectory.getChildFile("Factory");
        if (!factoryDir.exists() && !factoryDir.createDirectory()) {
            DBG("Failed to create Factory directory");
        }
            
        juce::File userDir = presetDirectory.getChildFile("User");
        if (!userDir.exists() && !userDir.createDirectory()) {
            DBG("Failed to create User directory");
        }
        
        clearError();
        return true;
    }
    catch (const std::exception& e) {
        reportError(ErrorCode::DirectoryCreationFailed,
                   "Exception creating directories: " + juce::String(e.what()));
        return false;
    }
}

void PresetManager::scanForPresets()
{
    const juce::ScopedWriteLock lock(presetsLock);
    
    try {
        presets.clear();
        
        if (!presetDirectory.exists()) {
            reportError(ErrorCode::DirectoryCreationFailed,
                       "Preset directory does not exist: " + presetDirectory.getFullPathName());
            return;
        }
        
        // Recursively scan the preset directory
        scanDirectory(presetDirectory, "");
        
        // Rebuild the folder hierarchy
        buildFolderHierarchy();
        
        clearError();
    }
    catch (const std::exception& e) {
        reportError(ErrorCode::FileReadFailed,
                   "Exception during preset scan: " + juce::String(e.what()));
    }
}

void PresetManager::clearFactoryPresets()
{
    const juce::ScopedWriteLock lock(presetsLock);
    
    for (auto it = presets.begin(); it != presets.end(); ) {
        if (it->second && it->second->category == "Factory") {
            it = presets.erase(it);
        } else {
            ++it;
        }
    }
}

bool PresetManager::savePreset(const juce::String& name, const juce::String& category)
{
    if (!validatePresetName(name)) {
        reportError(ErrorCode::InvalidPresetName, "Invalid preset name: " + name);
        return false;
    }
    
    if (!validateCategory(category)) {
        reportError(ErrorCode::InvalidCategory, "Invalid category: " + category);
        return false;
    }
    
    try {
        auto currentState = apvts.copyState();
        auto newPreset = std::make_unique<Preset>(name, category, currentState);
        
        if (!newPreset->isValid()) {
            reportError(ErrorCode::InvalidPresetData, "Invalid preset data");
            return false;
        }
        
        // Determine the target directory
        juce::File targetDir;
        
        if (category == "Factory" || category.startsWith("Factory/")) {
            if (category == "Factory") {
                targetDir = presetDirectory.getChildFile("Factory");
            } else {
                juce::String subfolderPath = category.substring(8);
                targetDir = presetDirectory.getChildFile("Factory");
                
                juce::StringArray folders = juce::StringArray::fromTokens(subfolderPath, "/", "");
                for (const auto& folder : folders) {
                    targetDir = targetDir.getChildFile(folder);
                    if (!targetDir.exists() && !targetDir.createDirectory()) {
                        reportError(ErrorCode::DirectoryCreationFailed,
                                   "Failed to create directory: " + targetDir.getFullPathName());
                        return false;
                    }
                }
            }
        } else {
            targetDir = presetDirectory.getChildFile("User");
            
            if (category != "User" && !category.isEmpty()) {
                juce::StringArray folders = juce::StringArray::fromTokens(category, "/", "");
                for (const auto& folder : folders) {
                    targetDir = targetDir.getChildFile(folder);
                    if (!targetDir.exists() && !targetDir.createDirectory()) {
                        reportError(ErrorCode::DirectoryCreationFailed,
                                   "Failed to create directory: " + targetDir.getFullPathName());
                        return false;
                    }
                }
            }
        }
        
        // Ensure directory exists
        if (!targetDir.exists() && !targetDir.createDirectory()) {
            reportError(ErrorCode::DirectoryCreationFailed,
                       "Failed to create preset directory at: " + targetDir.getFullPathName());
            return false;
        }
        
        // Save preset to file
        juce::File presetFile = targetDir.getChildFile(generateSafeFileName(name));
        
        if (savePresetToFile(*newPreset, presetFile)) {
            const juce::ScopedWriteLock lock(presetsLock);
            presets[name] = std::move(newPreset);
            clearError();
            return true;
        }
        
        return false;
    }
    catch (const std::exception& e) {
        reportError(ErrorCode::FileWriteFailed,
                   "Exception saving preset: " + juce::String(e.what()));
        return false;
    }
}

bool PresetManager::loadPreset(const juce::String& name)
{
    const juce::ScopedReadLock lock(presetsLock);
    
    auto it = presets.find(name);
    if (it == presets.end() || !it->second) {
        reportError(ErrorCode::PresetNotFound, "Preset not found: " + name);
        return false;
    }
    
    try {
        // Create a deep copy of the preset state
        juce::ValueTree presetCopy = it->second->state.createCopy();
        
        // Replace the entire state with the preset's state
        apvts.replaceState(presetCopy);
        
        // Update the current preset name
        currentPresetName = name;
        
        // Save a clean copy for modification detection
        cleanPresetState = presetCopy.createCopy();
        
        // Apply parameters in the correct order
        applyParametersInCorrectOrder();
        
        // Call the callback if set
        if (onPresetLoaded) {
            onPresetLoaded();
        }
        
        clearError();
        return true;
    }
    catch (const std::exception& e) {
        reportError(ErrorCode::InvalidPresetData,
                   "Exception loading preset: " + juce::String(e.what()));
        return false;
    }
}

bool PresetManager::isPresetModified() const noexcept
{
    try {
        juce::ValueTree currentState = apvts.copyState();
        return !currentState.isEquivalentTo(cleanPresetState);
    }
    catch (...) {
        return false;
    }
}

juce::String PresetManager::getDisplayedPresetName() const
{
    return currentPresetName;
}

juce::String PresetManager::getModifiedDisplayName() const
{
    try {
        juce::ValueTree currentState = apvts.copyState();
        if (!currentState.isEquivalentTo(cleanPresetState)) {
            return currentPresetName + "*";
        }
    }
    catch (...) {
        // Fall through to return unmodified name
    }
    return currentPresetName;
}

void PresetManager::initializeDefaultPresets()
{
    // This will be implemented by the preset loading system
}

juce::StringArray PresetManager::getPresetNames() const
{
    const juce::ScopedReadLock lock(presetsLock);
    
    juce::StringArray names;
    for (const auto& preset : presets) {
        if (preset.second && preset.second->isValid()) {
            names.add(preset.first);
        }
    }
    return names;
}

juce::StringArray PresetManager::getCategories() const
{
    const juce::ScopedReadLock lock(presetsLock);
    
    juce::StringArray categories;
    std::set<juce::String> uniqueCategories;
    
    for (const auto& preset : presets) {
        if (preset.second && preset.second->isValid()) {
            uniqueCategories.insert(preset.second->category);
        }
    }
    
    for (const auto& category : uniqueCategories) {
        categories.add(category);
    }
    
    return categories;
}

bool PresetManager::savePresetToFile(const Preset& preset, const juce::File& presetFile)
{
    try {
        // Validate the preset
        if (!preset.isValid()) {
            reportError(ErrorCode::InvalidPresetData, "Invalid preset data");
            return false;
        }
        
        // Convert state to XML
        auto xml = getXmlFromState(preset.state);
        if (!xml) {
            reportError(ErrorCode::InvalidPresetData, "Failed to create XML from preset state");
            return false;
        }
        
        // Add metadata
        xml->setAttribute("name", preset.name);
        xml->setAttribute("category", preset.category);
        xml->setAttribute("dateCreated", preset.dateCreated.toISO8601(true));
        xml->setAttribute("version", "1.0"); // Version for future compatibility
        
        // Save to file with error checking
        if (!xml->writeTo(presetFile)) {
            reportError(ErrorCode::FileWriteFailed,
                       "Failed to write preset file: " + presetFile.getFullPathName());
            return false;
        }
        
        // Verify the file was written
        if (!presetFile.exists() || presetFile.getSize() == 0) {
            reportError(ErrorCode::FileWriteFailed, "Preset file is empty or doesn't exist");
            return false;
        }
        
        clearError();
        return true;
    }
    catch (const std::exception& e) {
        reportError(ErrorCode::FileWriteFailed,
                   "Exception writing preset file: " + juce::String(e.what()));
        return false;
    }
}

bool PresetManager::loadPresetFromFile(const juce::File& file)
{
    if (!validatePresetFile(file)) {
        reportError(ErrorCode::FileReadFailed,
                   "Invalid preset file: " + file.getFullPathName());
        return false;
    }
    
    try {
        auto xml = juce::XmlDocument::parse(file);
        if (!xml) {
            reportError(ErrorCode::InvalidPresetData,
                       "Failed to parse XML from file: " + file.getFullPathName());
            return false;
        }
        
        // Extract metadata with validation
        juce::String name = xml->getStringAttribute("name").trim();
        if (name.isEmpty()) {
            name = file.getFileNameWithoutExtension();
        }
        name = Preset::sanitizeName(name);
        
        juce::String category = xml->getStringAttribute("category", "").trim();
        if (category.isEmpty()) {
            category = determineCategory(file);
        }
        category = Preset::sanitizeCategory(category);
        
        juce::Time dateCreated = juce::Time::fromISO8601(
            xml->getStringAttribute("dateCreated", juce::Time::getCurrentTime().toISO8601(true)));
        
        // Get the state
        juce::ValueTree state = getStateFromXml(*xml);
        if (!state.isValid()) {
            reportError(ErrorCode::InvalidPresetData,
                       "Invalid state in preset file: " + file.getFullPathName());
            return false;
        }
        
        // Create and store the preset
        auto preset = std::make_unique<Preset>(name, category, state, dateCreated);
        if (preset->isValid()) {
            const juce::ScopedWriteLock lock(presetsLock);
            presets[name] = std::move(preset);
            clearError();
            return true;
        } else {
            reportError(ErrorCode::InvalidPresetData, "Created preset is invalid");
            return false;
        }
    }
    catch (const std::exception& e) {
        reportError(ErrorCode::FileReadFailed,
                   "Exception loading preset file: " + juce::String(e.what()));
        return false;
    }
}

juce::String PresetManager::generateSafeFileName(const juce::String& name)
{
    // Sanitize the name for file system
    juce::String safeName = name.trim();
    
    // Remove invalid characters
    safeName = safeName.removeCharacters("\\/:*?\"<>|");
    
    // Replace spaces with underscores
    safeName = safeName.replace(" ", "_");
    
    // Limit length (leave room for extension)
    if (safeName.length() > 120) {
        safeName = safeName.substring(0, 120);
    }
    
    // Ensure not empty
    if (safeName.isEmpty()) {
        safeName = "Untitled";
    }
    
    return safeName + ".xml";
}

juce::ValueTree PresetManager::getStateFromXml(const juce::XmlElement& xml)
{
    try {
        return juce::ValueTree::fromXml(xml);
    }
    catch (const std::exception& e) {
        reportError(ErrorCode::InvalidPresetData,
                   "Failed to create ValueTree from XML: " + juce::String(e.what()));
        return {};
    }
}

std::unique_ptr<juce::XmlElement> PresetManager::getXmlFromState(const juce::ValueTree& state)
{
    try {
        if (!state.isValid()) {
            reportError(ErrorCode::InvalidPresetData, "Invalid ValueTree state");
            return nullptr;
        }
        return std::unique_ptr<juce::XmlElement>(state.createXml());
    }
    catch (const std::exception& e) {
        reportError(ErrorCode::InvalidPresetData,
                   "Failed to create XML from ValueTree: " + juce::String(e.what()));
        return nullptr;
    }
}

juce::StringArray PresetManager::getFactoryPresetNames() const
{
    const juce::ScopedReadLock lock(presetsLock);
    
    juce::StringArray names;
    for (const auto& presetPair : presets) {
        if (presetPair.second &&
            presetPair.second->isValid() &&
            presetPair.second->category.startsWith("Factory")) {
            names.add(presetPair.first);
        }
    }
    return names;
}

juce::StringArray PresetManager::getUserPresetNames() const
{
    const juce::ScopedReadLock lock(presetsLock);
    
    juce::StringArray names;
    for (const auto& presetPair : presets) {
        if (presetPair.second &&
            presetPair.second->isValid() &&
            !presetPair.second->category.startsWith("Factory")) {
            names.add(presetPair.first);
        }
    }
    return names;
}

juce::String PresetManager::getPresetCategory(const juce::String& presetName) const
{
    const juce::ScopedReadLock lock(presetsLock);
    
    auto it = presets.find(presetName);
    if (it != presets.end() && it->second) {
        return it->second->category;
    }
    return "User"; // Default to User category if not found
}

void PresetManager::applyParametersInCorrectOrder()
{
    try {
        // Get raw parameter values safely
        auto* syncParam = apvts.getRawParameterValue("lfoSync");
        auto* divisionParam = apvts.getRawParameterValue("lfoNoteDivision");
        auto* rateParam = apvts.getRawParameterValue("lfoRate");
        
        if (!syncParam || !divisionParam || !rateParam) {
            reportError(ErrorCode::ParameterError, "Missing required parameters");
            return;
        }
        
        // First, ensure sync parameter is set
        bool isInSync = syncParam->load() > 0.5f;
        if (auto* sync = apvts.getParameter("lfoSync")) {
            sync->setValueNotifyingHost(isInSync ? 1.0f : 0.0f);
        }
        
        // Set division parameter
        if (auto* division = apvts.getParameter("lfoNoteDivision")) {
            float divisionValue = juce::jlimit(0.0f, 5.0f, divisionParam->load());
            float normalizedValue = divisionValue / 5.0f;
            division->setValueNotifyingHost(normalizedValue);
        }
        
        // Set rate parameter
        if (auto* rate = apvts.getParameter("lfoRate")) {
            float rateValue = juce::jlimit(0.01f, 25.0f, rateParam->load());
            rate->setValueNotifyingHost(rate->convertTo0to1(rateValue));
        }
        
        clearError();
    }
    catch (const std::exception& e) {
        reportError(ErrorCode::ParameterError,
                   "Exception applying parameters: " + juce::String(e.what()));
    }
}

std::vector<juce::String> PresetManager::splitFolderPath(const juce::String& path)
{
    std::vector<juce::String> components;
    
    if (path.isEmpty()) {
        return components;
    }
    
    juce::StringArray tokens = juce::StringArray::fromTokens(path, "/", "");
    
    for (const auto& token : tokens) {
        if (token.isNotEmpty()) {
            components.push_back(token);
        }
    }
    
    return components;
}

void PresetManager::buildFolderHierarchy()
{
    // Clear existing hierarchy
    presetFolders.clear();
    
    // Create top-level folders
    presetFolders["Factory"] = PresetFolder{"Factory", {}, {}};
    presetFolders["User"] = PresetFolder{"User", {}, {}};
    
    // Process all presets
    for (const auto& preset : presets) {
        if (!preset.second || !preset.second->isValid()) {
            continue;
        }
        
        juce::String presetName = preset.first;
        juce::String category = preset.second->category;
        
        if (category == "Factory") {
            presetFolders["Factory"].addPreset(presetName);
        }
        else if (category.startsWith("Factory/")) {
            juce::String subfolderPath = category.substring(8);
            
            PresetFolder* currentFolder = &presetFolders["Factory"];
            auto pathComponents = splitFolderPath(subfolderPath);
            
            for (const auto& component : pathComponents) {
                currentFolder = &currentFolder->getOrCreateSubfolder(component);
            }
            
            currentFolder->addPreset(presetName);
        }
        else if (category == "User") {
            presetFolders["User"].addPreset(presetName);
        }
        else {
            // Handle user subcategories
            PresetFolder* currentFolder = &presetFolders["User"];
            auto pathComponents = splitFolderPath(category);
            
            for (const auto& component : pathComponents) {
                currentFolder = &currentFolder->getOrCreateSubfolder(component);
            }
            
            currentFolder->addPreset(presetName);
        }
    }
}

juce::StringArray PresetManager::getFactoryCategories() const
{
    juce::StringArray categories;
    
    auto it = presetFolders.find("Factory");
    if (it != presetFolders.end()) {
        categories.add("Factory");
        
        std::function<void(const PresetFolder&, const juce::String&)> traverseFolder =
            [&categories, &traverseFolder](const PresetFolder& folder, const juce::String& path)
        {
            for (const auto& subfolder : folder.subfolders) {
                juce::String newPath = path.isEmpty() ? subfolder.first : path + "/" + subfolder.first;
                categories.add("Factory/" + newPath);
                traverseFolder(subfolder.second, newPath);
            }
        };
        
        traverseFolder(it->second, "");
    }
    
    return categories;
}

juce::StringArray PresetManager::getPresetsInFolder(const juce::String& folderPath) const
{
    juce::StringArray result;
    
    if (folderPath == "Factory" || folderPath == "User") {
        auto it = presetFolders.find(folderPath);
        if (it != presetFolders.end()) {
            for (const auto& presetName : it->second.presets) {
                result.add(presetName);
            }
        }
        return result;
    }
    
    const PresetFolder* folder = getFolderByPath(folderPath);
    if (folder != nullptr) {
        for (const auto& presetName : folder->presets) {
            result.add(presetName);
        }
    }
    
    return result;
}

juce::String PresetManager::categoryToFolderPath(const juce::String& category) const
{
    return category;
}

const PresetManager::PresetFolder* PresetManager::getFolderByPath(const juce::String& path) const
{
    auto components = splitFolderPath(path);
    
    if (components.empty()) {
        return nullptr;
    }
    
    auto rootFolderIt = presetFolders.find(components[0]);
    if (rootFolderIt == presetFolders.end()) {
        return nullptr;
    }
    
    const PresetFolder* currentFolder = &rootFolderIt->second;
    
    for (size_t i = 1; i < components.size(); ++i) {
        auto subfolderIt = currentFolder->subfolders.find(components[i]);
        if (subfolderIt == currentFolder->subfolders.end()) {
            return nullptr;
        }
        currentFolder = &subfolderIt->second;
    }
    
    return currentFolder;
}

void PresetManager::scanDirectory(const juce::File& directory, const juce::String& categoryPrefix)
{
    if (!directory.exists() || !directory.isDirectory()) {
        return;
    }
    
    try {
        // Process all XML files in this directory
        for (const auto& file : directory.findChildFiles(juce::File::findFiles, false, "*.xml")) {
            if (validatePresetFile(file)) {
                loadPresetFromFile(file);
            }
        }
        
        // Recursively process subdirectories
        for (const auto& subdir : directory.findChildFiles(juce::File::findDirectories, false)) {
            juce::String category;
            
            if (subdir.getFileName() == "Factory" || subdir.getFileName() == "User") {
                category = subdir.getFileName();
            }
            else if (!categoryPrefix.isEmpty()) {
                category = categoryPrefix + "/" + subdir.getFileName();
            }
            else {
                category = subdir.getFileName();
            }
            
            scanDirectory(subdir, category);
        }
    }
    catch (const std::exception& e) {
        reportError(ErrorCode::FileReadFailed,
                   "Exception scanning directory: " + juce::String(e.what()));
    }
}

void PresetManager::setCustomPresetName(const juce::String& name)
{
    currentPresetName = Preset::sanitizeName(name);
}

juce::String PresetManager::determineCategory(const juce::File& file)
{
    juce::String relativePath = file.getParentDirectory().getRelativePathFrom(presetDirectory);
    
    if (relativePath.isEmpty() || relativePath == ".") {
        return "User";
    }
    
    return Preset::sanitizeCategory(relativePath);
}

// Validation methods
bool PresetManager::validatePresetName(const juce::String& name) const noexcept
{
    if (name.isEmpty() || name.length() > MAX_PRESET_NAME_LENGTH) {
        return false;
    }
    
    // Check for invalid characters
    const char* invalidChars = "\\/:*?\"<>|";
    for (int i = 0; i < name.length(); ++i) {
        if (juce::String(invalidChars).containsChar(name[i])) {
            return false;
        }
    }
    
    return true;
}

bool PresetManager::validateCategory(const juce::String& category) const noexcept
{
    if (category.isEmpty() || category.length() > MAX_CATEGORY_LENGTH) {
        return false;
    }
    
    // Allow forward slashes for hierarchy but not other path characters
    const char* invalidChars = "\\:*?\"<>|";
    for (int i = 0; i < category.length(); ++i) {
        if (juce::String(invalidChars).containsChar(category[i])) {
            return false;
        }
    }
    
    return true;
}

bool PresetManager::validatePresetFile(const juce::File& file) const noexcept
{
    if (!file.exists() || !file.hasFileExtension("xml")) {
        return false;
    }
    
    // Check file size
    if (file.getSize() > MAX_PRESET_FILE_SIZE || file.getSize() == 0) {
        return false;
    }
    
    // Check read permissions
    if (!file.hasReadAccess()) {
        return false;
    }
    
    return true;
}

void PresetManager::reportError(ErrorCode code, const juce::String& message) const noexcept
{
    lastError = code;
    lastErrorMessage = message;
    
    #if JUCE_DEBUG
    DBG("PresetManager Error [" + juce::String(static_cast<int>(code)) + "]: " + message);
    #endif
}
