   auto pRegistry = AudacityFileConfig::Create(
      {}, {}, FileNames::PluginRegistry());
   auto &registry = *pRegistry;

   // Clear pluginregistry.cfg (not audacity.cfg)
   registry.DeleteAll();