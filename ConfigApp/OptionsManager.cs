﻿using Shared;
using System.IO;

namespace ConfigApp
{
    public static class OptionsManager
    {
        public static OptionsFile ConfigFile { get; } = new OptionsFile("config.ini");
        public static OptionsFile TwitchFile { get; } = new OptionsFile("twitch.ini");
        public static OptionsFile EffectsFile { get; } = new OptionsFile("effects.ini");

        public static void ReadFiles()
        {
            ConfigFile.ReadFile();
            TwitchFile.ReadFile();
            EffectsFile.ReadFile();
        }

        public static void WriteFiles()
        {
            ConfigFile.WriteFile();
            TwitchFile.WriteFile();
            EffectsFile.WriteFile();
        }

        public static void ResetFiles()
        {
            // Exclude TwitchFile as that one is reset separately

            ConfigFile.ResetFile();
            EffectsFile.ResetFile();
        }
    }
}
