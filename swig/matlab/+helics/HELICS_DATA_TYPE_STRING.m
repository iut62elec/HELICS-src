function v = HELICS_DATA_TYPE_STRING()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 39);
  end
  v = vInitialized;
end
