(*
 * (C) Copyright IBM Corp. 2019.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *)

open Process

(** add_missing_protocol *)

(* alist -> alist' *)
let rec add_missing_protocol alist =
  List.fold_left
    (fun rslt k -> add_missing_protocol_helper rslt k)
    alist ["input"; "output"]

and add_missing_protocol_helper alist k =
  (* k = "input" | "output" | ... *)
  (*Printf.eprintf "[add_missing_protocol_helper] %s\n" (string_of_process (P_null, alist));*)
  assert (List.length (List.find_all (fun (k', _) -> k' = k) alist) <= 1);
  let proto_opt : protocol option = detect_protocol alist k in
    match proto_opt with
    | None -> alist
    | Some proto when List.mem_assoc k alist ->
	let alist' = List.remove_assoc k alist
	and inner =
	  match List.assoc k alist with
	  | Att_attributes alist' -> List.remove_assoc "protocol" alist'
	  | _ -> assert false
	in
	assert (not @@ List.mem_assoc k alist');
	[k, Att_attributes (["protocol", Att_protocol proto] @ inner)] @ alist'
    | Some proto ->
	[k, Att_attributes ["protocol", Att_protocol proto]] @ alist

and detect_protocol alist k =
  (* alist = ["input": [...], "output": [...], "protocol": p]
     k = "input" | "output" | ...
   *)
  let has_valid_protocol alist =
    if not (List.mem_assoc "protocol" alist) then false else
    match List.assoc "protocol" alist with
    | Att_protocol TP_undefined | Att_protocol (TP_unknown _) -> false
    | _ -> true
  in let attr_opt =
    match List.assoc_opt k alist with
    | Some (Att_attributes alist') when has_valid_protocol alist' ->
	Some (List.assoc "protocol" alist')
    | Some (Att_attributes alist') when k = "input" ->
	if List.mem_assoc "events" alist' then
	  Some (Att_protocol TP_none)
	else if List.mem_assoc "path" alist' then
	  Some (Att_protocol TP_none)
	else if has_valid_protocol alist then
	  Some (List.assoc "protocol" alist)
	else
	  None
    | Some (Att_attributes alist') ->
	if List.mem_assoc "path" alist' then
	  Some (Att_protocol TP_none)
	else if has_valid_protocol alist then
	  Some (List.assoc "protocol" alist)
	else
	  None
    | _ -> None
  in
  match attr_opt with Some (Att_protocol p) -> Some p | _ -> None

(** unfold_let *)

(* process -> process *)
let rec unfold_let (p : t) =
  (*Printf.eprintf "[unfold_let] %s\n" (string_of_process p);*)
  let sentinel =
    ["type", Att_name "scxml";
     "protocol", Att_protocol TP_undefined;
     "input",  Att_attributes ["protocol", Att_protocol TP_undefined];
     "output", Att_attributes ["protocol", Att_protocol TP_undefined]]
  and e, alist = p
  in unfold_let_rec [] (e, merge_attrs (add_missing_protocol alist) sentinel)

(* (string * process) list -> process -> process *)
and unfold_let_rec (bindings : (string * process) list) (e, attrs) =
  (*to_channel stderr (e, attrs);*)
  let attrs = add_missing_protocol attrs in
  match e with
  | P_var x when List.mem_assoc x bindings ->
      let e1, attrs1 = List.assoc x bindings
      in e1, add_missing_protocol (merge_attrs attrs1 attrs)
  | P_var x ->
      failwith ("[unfold_let_rec] undeclared variable: " ^ x)
  | P_let (bindings1, e1) ->
      let bindings2 = List.map (fun (x, p) -> x, unfold_let p) bindings1
      in unfold_let_rec (bindings2 @ bindings) (e1, attrs)
  | P_pcomp ps ->
      P_pcomp (List.map (unfold_let_rec bindings) ps), attrs
  | _ -> e, attrs

and merge_attrs (attrs1 : (string * process_attr) list) attrs2 =
  List.fold_left
    (fun rslt (k, attr) ->
      if not (List.mem_assoc k rslt) then rslt @ [k, attr] else
      match k with
      | "protocol" when not (List.mem_assoc k rslt) ->
	  rslt @ [k, attr]
      | "protocol" ->
	  let rslt' =
	    match List.assoc k rslt with
	    | Att_protocol TP_undefined -> (List.remove_assoc k rslt) @ [k, attr]
	    | _ -> rslt
	  in rslt'
      | "input" | "output" | "error" | "log" ->
	  let attrs11 = match List.assoc k rslt with Att_attributes alist -> alist | _ -> assert false
	  and attrs21 = match attr with Att_attributes alist -> alist | _ -> assert false
	  and rslt = List.remove_assoc k rslt
	  in [k, Att_attributes (merge_attrs_helper attrs11 attrs21)] @ rslt
      | _ -> rslt)
    attrs1 attrs2

and merge_attrs_helper (attrs1 : (string * process_attr) list) attrs2 =
  let proto_opt1 = List.assoc_opt "protocol" attrs1
  and proto_opt2 = List.assoc_opt "protocol" attrs2
  in
  match proto_opt1, proto_opt2 with
  | None, _ | _, None -> attrs1 @ attrs2
  | Some (Att_protocol TP_undefined), _ -> (List.remove_assoc "protocol" attrs1) @ attrs2
  | _ -> attrs1 @ List.remove_assoc "protocol" attrs2

(** flatten nested process *)

(* parallel composition *)
let rec flatten_pcomp (e, attrs) =
  (*Printf.eprintf "[flatten_pcomp] %s\n" (string_of_process (e, attrs));*)
  match e with
  | P_pcomp ps ->
      P_pcomp (List.map (flatten_pcomp_helper attrs) ps), attrs
  | P_let _ -> failwith "[flatten] invalid let: "
  | _ -> e, attrs

and flatten_pcomp_helper outer (e, inner) =
  let e', inner' = flatten_pcomp (e, inner)
  in e', merge_attrs (add_missing_protocol inner') outer

(** sort *)

let _id_count = ref 0
let gen_id () = incr _id_count; Att_int !_id_count

(* process -> process list *)
let assign_id (p : t) =
  let e, attrs = p in
  match e with
  | P_null | P_const _ | P_var _ ->
      [e, ["id", gen_id ()] @ attrs]
  | P_pcomp ps ->
      List.map
	(fun (e, attrs) ->
	  match e with
	  | P_null | P_const _ | P_var _ -> e, ["id", gen_id ()] @ attrs
	  | _ -> failwith "[assign_id]")
	ps
  | _ -> failwith "[assign_id]"

let rec sort p =
  let e, attrs = p
  in P_pcomp (List.sort proc_compare (assign_id p)), attrs

and proc_compare p1 p2 =
  let _, attrs1 = p1 and _, attrs2 = p2 in
  let get_id attrs =
    match List.assoc "id" attrs with Att_int id -> id | _ -> assert false
  and get_protocol attrs k =
    match List.assoc k attrs with
    | Att_attributes alist ->
	(match List.assoc "protocol" alist with Att_protocol p -> p | _ -> assert false)
    | _ -> assert false
  and fail_undefined p =
    failwith ("[proc_compare] undefined protocol: " ^ string_of_process p)
  in
  let id1, pin1, pout1 = get_id attrs1, get_protocol attrs1 "input", get_protocol attrs1 "output"
  and id2, pin2, pout2 = get_id attrs1, get_protocol attrs2 "input", get_protocol attrs2 "output"
  in
  if pin1 = TP_undefined || pout1 = TP_undefined then fail_undefined p1 else
  if pin2 = TP_undefined || pout2 = TP_undefined then fail_undefined p2 else

  match pin1, pin2 with
  | TP_mqtt, p when p <> TP_mqtt -> -1
  | p, TP_mqtt when p <> TP_mqtt -> +1
  | TP_mqtt, TP_mqtt ->
      let rslt =
	match pout1, pout2 with
	| TP_mqtt, p when p <> TP_mqtt -> +1
	| p, TP_mqtt when p <> TP_mqtt -> -1
	| _ -> compare id1 id2
      in rslt
  | _ ->
      compare id1 id2

(** canonicalize *)

let canonicalize (p : t) =
  p |> unfold_let |> flatten_pcomp |> sort

(** scriptize *)

let preamble =
  ["set -eu -o pipefail";
   "USER=${USER:-$(id -u -n)}";
   "test $(pgrep -c -u $USER scxmlrun) -eq 0 || { echo \"scxml running\"; exit 1; }"]

let postamble =
  ["while test $(pgrep -c -u $USER scxmlrun) -gt 0; do sleep 1s; done";
   "exit 0"]

let rec scriptize ?(verbose = false) oc (p : t) =
  let e, attrs = p in

  (* preamble *)
  output_string oc "# preamble\n";
  List.iter (fun line -> output_string oc line; output_string oc "\n") preamble;
  let _ =
    match List.assoc_opt "preamble" attrs with
    | Some (Att_strings lines) ->
	List.iter (fun line -> output_string oc line; output_string oc "\n") lines
    | None -> ()
  in
  
  (* processes *)
  let _ =
    match e with
    | P_pcomp ps when ps <> [] ->
	let n = List.length ps in
	Printf.fprintf oc "# run: %d process%s\n" n (if n = 1 then "" else "es");
	List.iteri (scriptize_helper ~verbose oc (List.length ps)) ps
    | _ -> failwith "[scriptize]"
  in

  (* postamble *)
  output_string oc "# postamble\n";
  let _ =
    match List.assoc_opt "postamble" attrs with
    | Some (Att_strings lines) ->
	List.iter (fun line -> output_string oc line; output_string oc "\n") lines
    | None -> ()
  in
  List.iter (fun line -> output_string oc line; output_string oc "\n") postamble

and scriptize_helper ?(verbose = false) oc nmax n p =
  if verbose then (output_string oc "# "; to_channel oc p; output_string oc "\n");
  let e, attrs = p in
  assert (List.mem_assoc "type" attrs);
  match List.assoc "type" attrs with
  | Att_name "scxml" -> scriptize_scxml oc (n + 1 < nmax) p
  | Att_name "script" -> scriptize_script oc (n + 1 < nmax) p
  | _ -> failwith "[scriptize_helper]"

and scriptize_scxml oc bg_job (e, attrs) =
  let scxml_file = match e with P_const (file :: _) -> file | _ -> assert false
  and attrs_in =
    match List.assoc "input" attrs with Att_attributes attrs -> attrs | _ -> assert false
  and attrs_out =
    match List.assoc "output" attrs with Att_attributes attrs -> attrs | _ -> assert false
  and pipe = ref None
  in
  let args_in : string =
    match List.assoc "protocol" attrs_in with
    | Att_protocol TP_none when List.mem_assoc "path" attrs_in ->
	let path =
	  match List.assoc "path" attrs_in with Att_string path -> path | _ -> assert false
	in
	"  -i " ^ path
    | Att_protocol TP_none when List.mem_assoc "events" attrs_in ->
	let events : Yojson.Safe.json list =
	  match List.assoc "events" attrs_in with Att_events events -> events | _ -> assert false
	in
	if events = [] then "" else
	let events : string =
	  List.fold_left (fun rslt ev -> rslt ^ Yojson.Safe.to_string ev ^ "\\n") "" events
	in
	pipe := Some ("echo -ne '" ^ events ^ "'");
	"  -i /dev/stdin"
    | Att_protocol TP_none ->
	failwith "[scriptize_scxml] no input source specified"
    | Att_protocol TP_mqtt when not (List.mem_assoc "mqtt_topic" attrs_in) ->
	failwith "[scriptize_scxml] topic unspecified"
    | Att_protocol TP_mqtt ->
	let topics =
	  match List.assoc "mqtt_topic" attrs_in with Att_strings topics -> topics | _ -> assert false
	in
	List.fold_left (fun rslt topic -> rslt ^ " --sub " ^ topic) "" topics
    | Att_protocol p ->
	invalid_arg ("[scriptize_scxml] invalid input protocol: " ^ string_of_protocol p)
  and args_out : string =
    match List.assoc "protocol" attrs_out with
    | Att_protocol TP_none when List.mem_assoc "path" attrs_out ->
	let path =
	  match List.assoc "path" attrs_out with Att_string path -> path | _ -> assert false
	in
	"  -o " ^ path
    | Att_protocol TP_none ->
	""
    | Att_protocol TP_mqtt when List.mem_assoc "mqtt_topic" attrs_out ->
	let topics =
	  match List.assoc "mqtt_topic" attrs_out with Att_strings topics -> topics | _ -> assert false
	in
	assert (List.length topics = 1);
	" --pub " ^ List.hd topics;
    | Att_protocol TP_mqtt ->
	" --mqtt"
    | Att_protocol p ->
	invalid_arg ("[scriptize_scxml] invalid output protocol: " ^ string_of_protocol p)
  in
  output_string oc "{ ";
  output_string oc (match !pipe with None -> "" | Some pipe -> pipe ^ " | ");
  output_string oc "scxmlrun "; output_string oc scxml_file;
  output_string oc args_in;
  output_string oc args_out;
  output_string oc "; }"; if bg_job then output_string oc " &";
  output_string oc "\n";
  ()

and scriptize_script oc bg_job (e, attrs) =
  let script = match e with P_const (script :: _) -> script | _ -> assert false
  and attrs_in =
    match List.assoc "input" attrs with Att_attributes attrs -> attrs | _ -> assert false
  and attrs_out =
    match List.assoc "output" attrs with Att_attributes attrs -> attrs | _ -> assert false
  in
  let source : string option =
    match List.assoc "protocol" attrs_in with
    | Att_protocol TP_none when List.mem_assoc "path" attrs_in ->
	let path : string option =
	  match List.assoc "path" attrs_in with
	  | Att_null -> None
	  | Att_string path -> Some ("cat " ^ path)
	  | _ -> assert false
	in
	path
    | Att_protocol TP_none when List.mem_assoc "events" attrs_in ->
	let events : Yojson.Safe.json list =
	  match List.assoc "events" attrs_in with Att_events events -> events | _ -> assert false
	in
	if events = [] then None else
	let events : string =
	  List.fold_left (fun rslt ev -> rslt ^ Yojson.Safe.to_string ev ^ "\\n") "" events
	in
	Some ("echo -ne '" ^ events ^ "'")
    | Att_protocol TP_none ->
	None
    | Att_protocol TP_mqtt ->
	None
    | Att_protocol p ->
	invalid_arg ("[scriptize_scriptl] invalid input protocol: " ^ string_of_protocol p)
  and sink =
    match List.assoc "protocol" attrs_out with
    | Att_protocol TP_none when List.mem_assoc "path" attrs_out ->
	let path =
	  match List.assoc "path" attrs_out with Att_string path -> path | _ -> assert false
	in
	Some path
    | Att_protocol TP_none ->
	None
    | Att_protocol TP_mqtt ->
	None
    | Att_protocol p ->
	invalid_arg ("[scriptize_scriptl] invalid output protocol: " ^ string_of_protocol p)
  in
  output_string oc "{ ";
  output_string oc (match source with None -> "" | Some cmd -> cmd ^ " | ");
  output_string oc ("{ " ^ script ^ "; }");
  output_string oc (match sink with None -> "" | Some cmd -> " > " ^ cmd);
  output_string oc "; }"; if bg_job then output_string oc " &";
  output_string oc "\n";
  ()
