<drac2>
#* recognize packs as argument
#* create a backup and subcommand undo
#* create coin pouch automatically if needed on coin change, using coins
#* make change up, by attempting to remove some bigger coins and then adding the change
#* remove whole bag with delta -1.. this might conflict (syntactically) with bag# negative index. but delta 1 will be add, 2 will be select #2. -1 will be remove -x push -(x-1) back on the queue
#*  removing it from the entire list will mess up the indexing, queue them for removal in the report of separate set
#* help sub command, minimal syntax with doing just !qb
#* solve 'ration' matching problem in the item database but especially in the bag
# *  Split and reorder all exact matches for priority: exact bag, set, item then partial bag set item
# *  For code reuse ~ prefix to support partial match, automatically requeue with partial if no exact match
# explicit + for bags always adds, never selects
# Remove from any/all bags (but selected first) (split up removal from addition?)
# support bag open none one all (current is like one, none is no report, all is add range of bags to report at start)
# $ prefix buys (automatically remove coins), but change? and -$ to sell?
# Weight summary and delta (with support for custom weight configuration from bag)
# Add capped removal items back to show in failed
# [x] delta before bag selection doesn't add multiple but select an indexed copy [1 is first -1 is last], add to help
# add recognized class weapons to a worn/equipped bag instead
# add ammo to ammo bags automatically
# support using coins not in pouch
# support extra_packs variable used by !bag


args=&ARGS&

if not args:
	return f'echo `{ctx.prefix}{ctx.alias} [<bag>] [+/-][amount] <item> [<background>] [<pack>] ...`'

bv='bags'
backup=get(bv)
bags=load_json(get(bv,'[]'))
item_table=load_json(get_gvar('19753bce-e2c1-42af-8c4f-baf56e2f6749'))	 # original !bag items for interop

# load configuration, prioroty: server, char, user, global
var_name='quickbag'
sv=get_svar(var_name,'')
config=load_json(sv if sv else get(var_name,get_gvar('71ff40cc-4c5f-4ae5-bdb6-32130f869090'))) # get_svar(var_name,get(var_name,get_gvar('71ff40cc-4c5f-4ae5-bdb6-32130f869090'))))

# TODO add to quickbag gvar and add svar override
purse_names=config.get('purses',[])
equipped_names=config.get('equipment',[])
ammo_containers=config.get('ammo',[])
containers=config.get('containers',[]) + equipped_names + ammo_containers + purse_names
specific_bags=purse_names + ammo_containers
sets=config.get('packs',{})
sets.update(config.get('backgrounds',{}))
sets={n.lower():c for n,c in sets.items()}
coins=config.get('coinRates',{})
short_words=config.get('forbidden')
item_list=[i for i in item_table.keys() if i not in sets.keys()]


# find the purse bag
coin_bags = [idx for idx,b in enumerate(bags) if b[0].lower() in purse_names]
coin_idx = coin_bags[0] if coin_bags else None

# convert arguments to quantified changes
delta=1
buy=False
bag_idx=None	# track selection using index for report
debug_break=None
dbg=0
if args[0].startswith('dbg='):
	debug_break=int(args.pop(0)[4:])

removed_bags=[]
debug=[]
report={}	# list of modified bags: {bagidx:{item_name:[amount deltas]}], could be bag index to resolve reuse
fail='fail'

while args:
	# debug break
	if debug_break:
		dbg+=1
		if dbg>=debug_break:
			return f'echo **Debug [{dbg}]: *{debug}* arguments remaining `{delta}` `{args}`'
	# debug.append(str(args))
	arg=args.pop(0)

	## Prefixes
	if arg[0]=='$':	# unimplemented
		buy=True
		arg=arg[1:]
	elif arg[0]=='-':
		delta=-delta
		arg=arg[1:]
	elif arg[0] == '+':
		arg = arg[1:]

	if delta==1 and arg.isnumeric():
		delta=int(arg)
		continue
	elif delta==-1 and arg.isnumeric():
		delta=-int(arg)
		continue

	if arg and arg[0]=='~':
		partial=True
		arg=arg[1:]
	else:
		partial=False

	if not arg:	# spaces after prefixes are allowed, just continue parsing next argument
		continue

	if arg in short_words:
		err(f'`{arg}` is not an item. Use "quotes" for items that consist of more than one word.')

	# TODO: could maybe pop next arg here maybe and remove all the delta=1, buy=False, continue

	# TODO: if 1 is +1 and arg is the name of an existing bag, then select that bag. If it's preceded by $ then add the bag to bags first

	if delta==0:
		continue

	item_name=arg.lower()
	# jank optimization:
	#  - just using item_name in string overfits,
	#  - using item_name in string.split() underfits (ration, clothes, traveler's)
	#  - extra list comprehension any([a.startsWith(item_name) for a in string.split()]) hits loop limit
	#  so: string.startsWith(item_name) or space_name in string
	space_name = f' {item_name}'

	# select to existing bags if the delta is 1 and arg
	if partial:
		existing_bags = [idx for idx,b in enumerate(bags) if idx not in removed_bags and (b[0].lower().startswith(item_name.lower()) or space_name in b[0].lower())]
	else:
		existing_bags = [idx for idx,b in enumerate(bags) if idx not in removed_bags and (item_name.lower()==b[0].lower())]
	# TODO: if delta=-1 could delete existing bag and everything in it
	if delta<0 and existing_bags:
		removed_bag_idx=existing_bags[0]
		removed_bag=bags[removed_bag_idx]
		removed_bags.append(removed_bag_idx)	# don't remove until the end
		diff = report.get(removed_bag_idx,{})
		for i,q in removed_bag[1].items():
			removed_bag[1][i]=0
			diff[i]=diff.get(i,[q])+[0]

		if removed_bag_idx==bag_idx:
			bag_idx=None
		report[removed_bag_idx]=diff
		debug.append(f'Remove Bag {removed_bag[0]}')
		delta+=1
		if delta:	# repeat remove next backpack
			args=[item_name]+args
		else:
			delta=1
		continue
	elif delta==1 and existing_bags:
		bag_idx=existing_bags[0]
		bag=bags[bag_idx]
		# TODO: what if it's the same bag? dict? what is it's not? index?
		# TODO: report
		debug.append(f'Select {bag[0]}')
		if not bag_idx in report:
			report[bag_idx]={ }
		continue


	## Create a new purse with all coins if needed
	if coin_idx is None and item_name in coins.keys() and purse_names:
		purse_name=purse_names[0].title()
		bags.append([purse_name, {coin:0 for coin in coins.keys()}])
		coin_idx=len(bags)-1
		# report[coin_idx]={coin:[0] for coin in coins.keys()}
		report[coin_idx]={purse_name:[1]}
		debug.append(f'Purse {purse_name}')

	## Coins : if an item is in the coin pouch, change the coins (don't remove the whole entry)
	if coin_idx is not None:
		money=bags[coin_idx][1]
		if item_name in money.keys():
			current=money.get(item_name,0)
			if -delta>current:
				# make chance:
				bigger_coins={c:r for c,r in coins.items() if r<coins[item_name]}
				if not bigger_coins:
					err(f'You don\'t have {-delta} {item_name}.')
				else:
					next_coin=list(bigger_coins.keys())[0]
					exchange_rate=int(round(coins[item_name]/coins[next_coin]))
					next_needed=int(ceil((-delta-current)/exchange_rate))
					change=next_needed*exchange_rate + delta
					if change:
						args=[next_coin, str(change), item_name]+args
					else:
						args=[next_coin]+args
					delta=-next_needed
					continue

			money[item_name]=current+delta

			# update report
			diff = report.get(coin_idx,{})
			diff[item_name]=diff.get(item_name,[])+[current, delta]
			report[coin_idx]=diff

			# TODO report[pouch_name].get(coin,[amount])
			debug.append(f'$ {delta} {item_name}')
			delta=1
			continue

	## Add: new bags
	if delta==1:
		if partial:
			new_bags=[nb for nb in containers if nb.startswith(item_name) or space_name in nb]
		else:
			new_bags=[nb for nb in containers if item_name==nb]
		if new_bags:
			new_bag=new_bags[0].title()
			bags.append([new_bag,{}])
			args=[new_bag]+args	# queue switching to this bag

			# add the bag's name in its report to remember that it's new
			report[len(bags)-1]={new_bag:[1]}
			debug.append(f'Bag {new_bag}')
			delta=1
			continue

	# Default bag: about to add items to  the current bag, if there is no current bag, select one
	if bag_idx is None:
		default_bags=[b[0] for i,b in enumerate(bags) if i not in removed_bags and (b[0].lower() not in specific_bags)]
		# queue switching to the default bag explicitly and adding the current item. create arg basic one if none found
		if default_bags:
			default_bag=default_bags[0]
			args=[default_bag ,str(delta),arg]+args
			debug.append(f'Default {default_bag}')
			delta=1
			continue
		elif containers and delta>0:
			default_bag = containers[0]
			debug.append(f'Default* {default_bag}')
			args=[default_bag ,str(delta),arg]+args
			delta=1
			continue
		else:	# will fail to remove from no bag
			pass
	else:
		###  ITEMS
		bag = bags[bag_idx]
		# Modify: find items already in the current bag, always use partial match first
		if partial:
			mod_items=[]
		else:
			mod_items = [n for n in bag[1].keys() if n.lower().startswith(item_name) or space_name in n.lower()]
		if mod_items:
			mod_item=mod_items[0]
			current=bag[1].get(mod_item)
			amount=max(0,current+delta)
			if amount:
				bag[1][mod_item]=amount
				debug.append(f'Adjust {current}+{delta}={amount} x {mod_item}')
			else:
				bag[1].pop(mod_item)
				debug.append(f'Remove {current} x {mod_item}')

			# update the item's diff in the report
			diff=report.get(bag_idx,{})
			diff[mod_item]=diff.get(mod_item,[current])+[current-amount]
			report[bag_idx]=diff

			delta=1
			continue

		# NEW known items
		if partial:
			new_items += [n for n in item_list if n.startswith(item_name) or space_name in n.lower()]
		else:
			new_items = [n for n in item_list if item_name == n]
		if delta>0 and new_items:
			new_item=new_items[0].title().replace("'S","'s")
			bag[1][new_item]=delta
			debug.append(f'Add {delta} x {new_item}')
			# update the item's diff in the report
			diff=report.get(bag_idx,{})
			diff[new_item]=[0,delta]
			report[bag_idx]=diff

			delta=1
			continue

		# Sets: add all of a set's contents to the arguments and parse as normal
		if partial:
			item_set += [n for n in sets.keys() if n.startswith(item_name) or space_name in n.lower()]
		else:
			item_set = [n for n in sets.keys() if item_name==n]
		if delta>0 and item_set:
			item_set=item_set[0]
			set_items=sets[item_set]
			added_args=[]
			for item_name,q in set_items.items():
				added_args+=[str(q),item_name]
			args=added_args * delta + args
			debug.append(f'Set {item_set}')
			delta=1
			continue

		# try again with partial prefix
		if not partial:
			args=[f'~{arg}'] + args
			continue

		# Lowest priority: unrecognized item
		if delta>0:
			new_item=item_name.title().replace("'S","'s")
			bag[1][new_item]=delta
			debug.append(f'Unknown {delta} x {new_item}')
			diff=report.get(bag_idx,{})
			diff[new_item]=[0,delta]
			report[bag_idx]=diff
			delta=1
			continue

	debug.append('f Missing {-delta} {item_name}')
	diff=report.get('fail',{})
	diff[item_name]=diff.get(item_name,[])+[delta]
	report['fail']=diff

	delta=1

# format the bags
nl='\n'
fields=''
if  report:
	for idx,changes in report.items():
		if idx==fail:
			bag_name="Failed"
			bag=None
		else:
			bag=bags[idx]
			bag_name=bag[0]
		if bag_name in changes:
			changes.pop(bag_name)
			bag_name+='+'
		if idx in removed_bags:
			bag_name+='-'

		contents=bag[1] if bag else {}
		items=[]
		if idx==coin_idx:
			# format as coins: no plural, no removal, no new items
			for coin,q in contents.items():
				if coin in changes:
					items.append(f'~~{changes[coin][0]}~~ {q} {coin}')
				else:
					items.append(f'{q} {coin}')
		else:
			for item_name,q in contents.items():
				plural_name=item_name if item_name[-1]=='s' else item_name+'s'

				item_desc= f'{q} x {plural_name}' if q>1 else item_name	# TODO: better plural
				if item_name in changes:
					original_amount=changes[item_name][0]
					if original_amount==0:
						items.append('+' + item_desc)
					else:
						items.append(f'~~{original_amount}~~ '+ item_desc)
				else:	# unchanged item
					items.append(item_desc)
			for item_name,diff in changes.items():
				plural_name=item_name if item_name[-1]=='s' else item_name+'s'
				if item_name not in contents.keys():
					original_amount=diff[0]
					items.append(f'~~{original_amount} x {plural_name}~~' if original_amount!=1 else f'~~{item_name}~~')
		if not items:
			items=['Empty']
		fields+=f' -f "{bag_name}|{nl.join(items)}|inline"'


if debug_break:
	fields+=f' -f "Debug [{dbg}/{debug_break}]|{", ".join(debug)}"'

# remove bags after indices are no longer referenced
bag_idx=None
report=[]
bags=[b for i,b in enumerate(bags) if i not in removed_bags]

#backup
if backup:
	character().set_cvar('bag_backup', backup)

#update the variable
character().set_cvar(bv,dump_json(bags))
possessive=f'{name}\'' if name[-1]=='s' else f'{name}\'s'

return f'embed -title "{possessive} bags" -thumb https://images2.imgbox.com/69/c2/Fe3klotA_o.png {fields}'
</drac2>
