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

type process =
  process_exp * (string * process_attr) list

and process_exp =
  | P_null
        (** sentinel *)
  | P_const of string list
	(** path to a file, and extra info (if any) *)
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
	(** primarily for mqtt topics *)
  | Att_name of string
	(** symbol like 'scxml' *)

  | Att_protocol of protocol
  | Att_events of Yojson.Safe.json list
  | Att_attributes of (string * process_attr) list

  | Att_unknown of Yojson.Safe.json
	(** error *)

and protocol =
    (** network transport protocol *)
  | TP_undefined
	(** sentinel, indicating it needs to be resolved later *)
  | TP_none
	(** no network transport is involved *)
  | TP_http
  | TP_mqtt

  | TP_unknown of string
	(** error *)

type t = process

(** parsing/printing *)

val from_channel : in_channel -> process

val to_channel : ?format : string -> out_channel -> process -> unit

val string_of_process : ?format : string -> process -> string

val string_of_protocol : protocol -> string

(** ppx-generated *)
        
val pp_process : Format.formatter -> process -> unit
val show_process : process -> string

val process_of_yojson : Yojson.Safe.json -> (process, string) Result.result
val process_to_yojson : process ->  Yojson.Safe.json
