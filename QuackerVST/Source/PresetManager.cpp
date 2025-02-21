/*
  ==============================================================================

    PresetManager.cpp
    Created: 21 Feb 2025 7:34:15pm
    Author:  Deivids Hvostovs

  ==============================================================================
*/

#include "PresetManager.h"

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts)
    : apvts(apvts)
{
    // Set up preset directory in the user's documents folder
    juce::File documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    presetDirectory = documentsDir.getChildFile("Quacker/Presets");
    createPresetDirectory();
    scanForPresets();
}

PresetManager::~PresetManager()
{
}

void PresetManager::createPresetDirectory()
{
    if (!presetDirectory.exists())
    {
        const bool dirCreated = presetDirectory.createDirectory();
        if (!dirCreated)
        {
            // Handle directory creation failure
            juce::Logger::writeToLog("Failed to create preset directory at: " + presetDirectory.getFullPathName());
        }
    }
}

void PresetManager::scanForPresets()
{
    // Clear existing presets
    presets.clear();
    
    // Scan directory for preset files
    for (const auto& file : presetDirectory.findChildFiles(
        juce::File::findFiles, false, "*.xml"))
    {
        loadPresetFromFile(file);
    }
}

void PresetManager::clearFactoryPresets()
{
    for (auto it = presets.begin(); it != presets.end(); )
    {
        if (it->second->category == "Factory")
            it = presets.erase(it);
        else
            ++it;
    }
}

bool PresetManager::savePreset(const juce::String& name, const juce::String& category)
{
    // Create a new preset from current state
    auto currentState = apvts.copyState();
    auto newPreset = std::make_unique<Preset>(name, category, currentState);
    
    // Save to file
    if (savePresetToFile(*newPreset))
    {
        // Add to our preset map
        presets[name] = std::move(newPreset);
        return true;
    }
    
    return false;
}

bool PresetManager::loadPreset(const juce::String& name)
{
    auto it = presets.find(name);
    if (it != presets.end())
    {
        // Found the preset, load it
        apvts.replaceState(it->second->state);
        return true;
    }
    
    return false;
}


void PresetManager::initializeDefaultPresets()
{
    // Here we'll add factory presets
    // This will be implemented in the next step
}

juce::StringArray PresetManager::getPresetNames() const
{
    juce::StringArray names;
    for (const auto& preset : presets)
    {
        names.add(preset.first);
    }
    return names;
}

juce::StringArray PresetManager::getCategories() const
{
    juce::StringArray categories;
    std::set<juce::String> uniqueCategories;
    
    for (const auto& preset : presets)
    {
        uniqueCategories.insert(preset.second->category);
    }
    
    for (const auto& category : uniqueCategories)
    {
        categories.add(category);
    }
    
    return categories;
}

bool PresetManager::savePresetToFile(const Preset& preset)
{
    // Convert state to XML
    if (auto xml = getXmlFromState(preset.state))
    {
        // Add metadata
        xml->setAttribute("name", preset.name);
        xml->setAttribute("category", preset.category);
        xml->setAttribute("dateCreated", preset.dateCreated.toISO8601(true));
        
        // Generate safe filename and save
        juce::File presetFile = presetDirectory.getChildFile(generateSafeFileName(preset.name));
        if (xml->writeTo(presetFile))
        {
            return true;
        }
    }
    
    return false;
}

void PresetManager::loadPresetFromFile(const juce::File& file)
{
    if (auto xml = juce::XmlDocument::parse(file))
    {
        // Extract metadata
        juce::String name = xml->getStringAttribute("name");
        juce::String category = xml->getStringAttribute("category", "User");
        juce::Time dateCreated = juce::Time::fromISO8601(xml->getStringAttribute("dateCreated"));
        
        // Get the state
        juce::ValueTree state = getStateFromXml(*xml);
        
        // Create and store the preset
        presets[name] = std::make_unique<Preset>(name, category, state, dateCreated);
    }
}

juce::String PresetManager::generateSafeFileName(const juce::String& name)
{
    // Remove invalid characters and replace spaces with underscores
    juce::String safeName = name.removeCharacters("\\/:*?\"<>|");
    safeName = safeName.replace(" ", "_");
    return safeName + ".xml";
}

juce::ValueTree PresetManager::getStateFromXml(const juce::XmlElement& xml)
{
    return juce::ValueTree::fromXml(xml);
}

std::unique_ptr<juce::XmlElement> PresetManager::getXmlFromState(const juce::ValueTree& state)
{
    return std::unique_ptr<juce::XmlElement>(state.createXml());
}


juce::StringArray PresetManager::getFactoryPresetNames() const
{
    juce::StringArray names;
    for (const auto& presetPair : presets)
    {
        if (presetPair.second->category == "Factory")
            names.add(presetPair.first);
    }
    return names;
}

juce::StringArray PresetManager::getUserPresetNames() const
{
    juce::StringArray names;
    for (const auto& presetPair : presets)
    {
        if (presetPair.second->category != "Factory")
            names.add(presetPair.first);
    }
    return names;
}


