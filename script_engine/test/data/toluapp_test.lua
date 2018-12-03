n = c_pet.AddPet(1, 2, 3)
print('n: ' .. n)

print('global and enum: ' .. c_pet.g_pet_prop_name_table[c_pet.PET_PROP_CID] .. ', ' .. c_pet.g_pet_prop_name_table[c_pet.PET_PROP_TYPE])

n = c_form.GetDefaultForm(0)
print('n: ' .. n)
