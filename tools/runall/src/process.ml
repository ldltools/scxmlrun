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

open Yojson.Safe

(** ppx_deriving_show *)

let pp_process _ p = ()
let show_process _ = ""
let show_process_exp _ = ""

let pp_json fmt json =
  ()

(** ppx_deriving_yojson *)
(*
let process_of_yojson j = failwith "[process_of_yojson]"
let process_to_yojson p = failwith "[process_to_yojson]"
 *)
let json_to_yojson = fun j -> j
let json_of_yojson j = Result.Ok j

(** process type *)

type process =
  process_exp * (string * process_attr) list

and process_exp =
  | P_null
  | P_const of string list
	(** path to a file, and its type (scxml, etc) *)
  | P_var of string
  | P_let of (string * process) list * process_exp
	(** let-binding *)
  | P_pcomp of process list
	(** parallel composition *)

and process_attr =
  | Att_null
  | Att_int of int
  | Att_bool of bool
  | Att_string of string
  | Att_strings of string list
  | Att_name of string

  | Att_protocol of protocol
  | Att_events of Yojson.Safe.json list
  | Att_attributes of (string * process_attr) list

  | Att_unknown of Yojson.Safe.json

and protocol =
  | TP_undefined
  | TP_none
  | TP_http
  | TP_mqtt
  | TP_unknown of string

(*[@@deriving show, yojson]*)
[@@deriving yojson]

type t = process

(** parsing *)

(* parse : json -> process *)
let rec parse (json : json) =
  match json with
  | `Null -> P_null, []
  | `Bool b -> P_null, ["const", Att_bool b]
  | `Int i -> P_null, ["const", Att_int i]
  | `String str -> P_null, ["const", Att_string str]

  | `Assoc alist when List.mem_assoc "process" alist ->
      let e, alist1 = parse_process @@ List.assoc "process" alist
      and alist2 = List.map parse_attr @@ List.remove_assoc "process" alist
      in e, alist1 @ alist2

  | _ ->
      failwith ("[parse] " ^ to_string json)

(* parse_process : json -> process
   p appears as: {"process": p} in the source json
 *)
and parse_process json =
  match json with
  | `Assoc alist when List.mem_assoc "variable" alist ->
      (* let-binding *)
      (* alist = [("variable", bindings); ("process", p)] *)
      let bindings = List.assoc "variable" alist
      and body = List.assoc "process" alist
      and rest = List.remove_assoc "variable" alist |> List.remove_assoc "process"
      in parse_process_let (bindings, body) rest
  | `Assoc alist when List.mem_assoc "name" alist ->
      (* variable reference *)
      let name =
	match List.assoc "name" alist with `String name -> name | _ -> assert false
      and rest = List.remove_assoc "name" alist
      in P_var name, List.map parse_attr rest
  | `Assoc alist ->
      (* const *)
      let k, v =
	match List.find_opt (fun (k, _) -> List.mem k ["path"; "script"]) alist with
	| Some (k, v) -> k, v
	| _ -> failwith ("[parse_process] " ^ to_string json)
      in parse_process_const (k, v) (List.remove_assoc k alist)
  | `List seq ->
      (* parallel composition *)
      P_pcomp (List.map parse seq), []
  | _ ->
      failwith ("[parse_process] " ^ to_string json)

and parse_process_const (k, v) alist_j =
  match k, v with
  | "path", `String path ->
      P_const [path], ["type", Att_name "scxml"] @ List.map parse_attr alist_j
  | "script", `String script ->
      P_const [script], ["type", Att_name "script"] @ List.map parse_attr alist_j
  | _ ->
      failwith ("[parse_process_const] " ^ k)

and parse_process_let (bindings_j, body_j) alist_j =
  let bindings : (string * process) list =
    let gen_binding alist =
      (* alist = ["name", n; "process", p] *)
      assert (List.mem_assoc "name" alist);
      assert (not (List.mem_assoc "process" alist && List.mem_assoc "value" alist));
      let name : string =
	match List.assoc "name" alist with
	| `String name -> name
	| _ -> failwith "[parse_process_let]"
      and p : process =
	if List.mem_assoc "process" alist then
	  parse_process @@ List.assoc "process" alist
	else if List.mem_assoc "value" alist then
	  parse @@ List.assoc "value" alist
	else
	  invalid_arg ("[parse_process_let] " ^ to_string (`Assoc alist))
      in name, p
    in
    match bindings_j with
    | `Assoc alist ->
	(* alist = ["name": n; "process": p] *)
	[gen_binding alist]
    | `List seq ->
	(* seq = [{"name": n; "process": p}; ...] *)
	List.fold_left
	  (fun rslt json ->
	    match json with
	    | `Assoc alist -> rslt @ [gen_binding alist]
	    | _ -> failwith "[parse_process_let]")
	  [] seq
    | _ ->
	failwith ("[parse_process_let] " ^ to_string bindings_j)
  in let body, alist1 =
    parse_process body_j
  in let alist2 =
    List.map parse_attr @@ List.remove_assoc "process" alist_j
  in
  P_let (bindings, body), alist1 @ alist2

(* string * json -> string * process_attr *)
and parse_attr (k, v) =
  let attr : process_attr =
    match k, v with
    | "type", `String ty -> Att_name ty
    | "protocol", `String proto ->
	let attr =
	  match proto with
	  | "none" -> Att_protocol TP_none
	  | "mqtt" -> Att_protocol TP_mqtt
	  | "http" -> Att_protocol TP_http
	  | _ -> Att_protocol (TP_unknown proto)
	in attr

    | "events", `Null -> Att_events []
    | "events", `List events -> Att_events events

    | "path", `Null -> Att_string "/dev/null"
    | "path", `String path -> Att_string path

    | "mqtt_host", `String host -> Att_string host
    | "mqtt_topic", `String topic
    | "topic", `String topic ->
	let topics = List.filter (fun str -> str <> "") (String.split_on_char ' ' topic)
	in Att_strings topics
    | "topic", `List topics
    | "topics", `List topics ->
	Att_strings (List.map (function `String topic -> topic | _ -> assert false) topics)

    | "preamble", `String line | "postamble", `String line ->
	Att_strings [line]
    | "preamble", `List seq | "postamble", `List seq ->
	let lines =
	  List.map
	    (function `String line -> line | _ -> invalid_arg ("[parse_attr] error: " ^ k))
	    seq
	in Att_strings lines

    | _, `Assoc alist when List.mem k ["input"; "output"; "error"; "log"] ->
	Att_attributes (List.map parse_attr alist)
    | _ ->
	Att_unknown v
  and k' =
    match k with
    | "topic" | "topics" -> "mqtt_topic"
    | _ -> k
  in k', attr

let from_channel ic =
  from_channel ic |> parse

(** printing *)

let rec print_process ?(fancy = false) (out : string -> unit) (p : t) =
  print_process_rec ~fancy out 0 p

and print_process_rec ?(fancy = false) (out : string -> unit) n (e, alist) =
  match alist with
  | [] ->
      print_process_exp ~fancy out n e
  | _ ->
      out "("; print_process_exp out ~fancy n e; out ")";
      print_attrs out alist

and print_process_exp ?(fancy = false) (out : string -> unit) n (e : process_exp) =
  match e with
  | P_null -> out "null"
  | P_const (hd :: _) -> out ("const(" ^ hd ^ ")")
  | P_var name -> out name
  | P_pcomp (p :: rest) ->
      print_process_rec out n p;
      List.iter
	(fun p ->
	  if fancy then
	    (out "\n"; for i = 1 to n do out " " done; out "|| ")
	  else
	    out " || ";
	  print_process_rec ~fancy out (n + 3) p)
	rest
  | P_let ((x, p) :: bindings, body) ->
      out "let "; out x; out " = "; print_process_rec ~fancy out n p;
      List.iter
	(fun (x, p) ->
	  out " and "; out x; out " = "; print_process_rec ~fancy out n p)
	bindings;
      out " in ";
      print_process_exp ~fancy out n body
  | _ ->
      failwith ("[print_process_exp] " ^ show_process_exp e)

and print_attrs out alist =
  match alist with
  | [] -> ()
  | attr :: rest ->
      out "["; print_attr out attr;
      List.iter (fun attr -> out ", "; print_attr out attr) rest;
      out "]"

and print_attr out (k, attr) =
  out k; out " = ";
  match attr with
  | Att_int i ->
      out (string_of_int i)
  | Att_name x ->
      out x
  | Att_string str ->
      out "\""; out str; out "\""
  | Att_strings (topic :: rest) ->
      out "[\""; out topic; out "\"";
      List.iter (fun topic -> out ", \""; out topic; out "\"") rest;
      out "]"

  | Att_protocol p ->
      out (string_of_protocol p)
  | Att_events (ev :: rest) ->
      out "["; out (to_string ev);
      List.iter (fun ev -> out ", "; out (to_string ev)) rest;
      out "]"
  | Att_attributes alist ->
      print_attrs out alist

  | Att_unknown v ->
      out "unknown("; out k; out ") = ";
      out (to_string v)
  | _ -> failwith ("[print_attr] " ^ k)

and string_of_protocol = function
  | TP_undefined -> "undefined"
  | TP_none -> "none"
  | TP_mqtt -> "mqtt"
  | TP_http -> "http"
  | TP_unknown p -> "unknown(\"" ^ p ^ "\")"

let string_of_process ?(format = "json") (p : t) =
  match format with
  | "ppx_yojson" | "yojson" ->
      process_to_yojson p |> to_string
  | "fancy" | _ ->
      let rslt = ref "" in print_process (fun str -> rslt := !rslt ^ str) p; !rslt

let to_channel ?(format = "unspecified") oc (p : t) =
  match format with
(*
  | "ppx_caml" | "caml" ->
      output_string oc @@ show_process p
*)
  | "ppx_yojson" | "yojson" ->
      process_to_yojson p |> to_channel oc
  | "fancy" | "unspecified" ->
      print_process (output_string oc) p
  | _ ->
      invalid_arg ("[to_channel] unknown format: " ^ format)
