{{cc='DM Inspiration'}}
{{f'adv -phrase "DM Inspiration" {character().mod_cc(cc,-1)} -f "{cc}"|"{cc_str(cc)}"' if character().cc_exists(cc) and character().get_cc(cc)>0 else f'-f "{cc}"|"Not Available" ' }}