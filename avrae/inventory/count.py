<drac2>
## TODO:
# Still partial match unrecognized items (eg superior healing potions) even if matching exact
# other categories like `ammo`?
# specify bag name doesn't count bag but that bag's contents?
# coins compact mode

bv='bags'
bags=load_json(get(bv,'[]'))
var_name='quickbag'
config=load_json(get_gvar(get_svar(var_name,'71ff40cc-4c5f-4ae5-bdb6-32130f869090')))
item_table=config.get('items',load_json(get_gvar('19753bce-e2c1-42af-8c4f-baf56e2f6749')))
coins=config.get('coinRates',{})
coin_list=list(coins.keys())
if coin_list and coins[coin_list[0]]>coins[coin_list[-1]]:
	coin_list.reverse()	# if it's sorted cheap (high rate per 1) to expensive (low rate per 1), reverse it. assume it's sorted at least
item_table.update({c:dict(weight=0.02, cost=f'{1}{c}', plural=c) for c in coins.keys()})

# use bagSettings
bag_settings=load_json(get('bagSettings','{}'))
item_table.update(bag_settings.get('customWeights'))

args=&ARGS&
if not args:
	args='*'

# convert partial arguments (if possible) to real item names
items=dict()	# a dict by argument with a list of all matched items
for arg in args:
	if arg=='*':	# * means all owned item types
		owned_items = {bn for bn, _ in bags}
		for _, bc in bags:
			owned_items.update(bc.keys())
		items[arg]=[i.lower() for i in owned_items]
	elif arg=='$':	# $ means owned coins
		items[arg]=coin_list
	else:	# else owned items from database
		arg=arg.lower()
		# fuzzy match argument with database
		item_names = [arg] if arg in item_table else []
		if not item_names:
			item_names = [n.lower() for n in item_table.keys() if any(np.startswith(arg) for np in n.lower().split())]
		#	if not item_names:
		#		item_names=[n for n in item_table.keys() if arg in n.lower()]
		for _,b in bags:
			item_names+=[i.lower() for i in b.keys() if arg.lower() in i.lower() and not any(i.lower() in n for n in item_names)]
		if item_names:
			items[arg]=item_names
		else:
			items[arg]=[arg]

# count owned items
owned=dict()
containers={""}
containers.remove("")
purse = character().coinpurse.get_coins()
for item_names in items.values():
	arg_owned=dict()
	for item_name in item_names:
		quantity = 0
		if bag_quant:={container:q for container,bag in bags for i,q in bag.items() if i.lower()==item_name}:
			quantity = sum(bag_quant.values())
			containers.update({b for b in bag_quant.keys()})
			quantity+=sum(1 for bn,_ in bags if item_name==bn.lower())
		elif bag_quant:={container:q for container,bag in bags for i,q in bag.items() if item_name in i.lower()}:
			quantity=sum(bag_quant.values())
			containers.update({b for b in bag_quant.keys()})
			quantity+=sum(1 for bn,_ in bags if item_name in bn.lower())
		# add coins from purse
		if coin_quant:=purse.get(item_name,0):
			quantity += coin_quant
			containers.add("Coinpurse")

		arg_owned[item_name] = owned.get(item_name, 0) + quantity

	# clear mismatched items
	if sum(arg_owned.values()):
		arg_owned={a:q for a,q in arg_owned.items() if q>0}
	elif arg_owned:	 # only add a single mismatched unowned item per argument
		first=list(arg_owned.keys())[0]
		arg_owned[first]=0
	# add items to total owned
	for i,q in arg_owned.items():
		owned[i]=owned.get(i,0)+q

# compute weight
total_weight = sum(q * item_table.get(n,{}).get('weight',0) for n,q in owned.items())
# compute value TODO: split into coins and stuff
coin_cost=dict()
for c in coin_list:
	coin_cost[c]=sum(q * int(cost[:-len(c)]) / i.get('bundle',1)  for n,q in owned.items() if (cost:=(i:=item_table.get(n,{})).get('cost','')).endswith(c))
for idx,c in enumerate(coin_list[:-1]):
	int_cost=int(coin_cost[c])
	exchange_coin=coin_list[idx+1]
	frac_rate=coins[exchange_coin]/coins[c]
	frac_cost = coin_cost[c] - int_cost	
	coin_cost[exchange_coin]+=frac_cost*frac_rate
	coin_cost[c]=int_cost

# plural
owned={(n if q<=1 else item_table.get(n,{}).get('plural', n if n[-1]=='s' else f'{n}s')):q for n,q in owned.items()}
# capitalize
owned={(n.title().replace("'S","'s") if not n in coin_list else n):q for n,q in owned.items()}
# format amounts
result='\n'.join(f'• {q:,}x {n}' if q else f'No {n}' for n,q in owned.items())
if containers:
	result+=f'\n\tin {get("their","their")} {", ".join(containers)}.'
# format costs
total_cost=' '.join(f'{int(q):,} {c}' for c,q in coin_cost.items() if q>0)
if not total_cost:
	total_cost='None'

footer=f'Total Weight: {round(total_weight)} lbs - Total Value: {total_cost}'
return f'embed -title "{name} has" -thumb https://images2.imgbox.com/69/c2/Fe3klotA_o.png -desc "{result}" -footer "{footer}" -color {color}'
</drac2>
