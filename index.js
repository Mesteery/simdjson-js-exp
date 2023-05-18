import pkg from 'benchmark';
const { Benchmark } = pkg;
import { __wasm as wasm } from './pkg/simdjson_ops.js';

var suite = new Benchmark.Suite();
const data = String.raw`{"jk_host":"jenkinshost100.my-domain-name.net","class_name":"ID\\Shared\\Logging\\Logger","logger_name":"important_logs_go_here!!","cgi_tte_ms":"3173.81","start_timestamp":1532362756,"user_agent_device":"Desktop","slush":"facility=important_logs_generated.gelf\ngelf_version=1.0\nfile=NA\nanother_value_includ=205894\nwe_got_some_one=1\n@version=1\nlog_length=1285\nline=0\nbools_are_an_awesome_thingy=0\nuser_agent_version=\nblubbergnaw_datas=somehostname.and.a.net\npower2=65360\nbot_flag=0\nnumbers=28921\na_host=somehostname.and.a.net\nwe_can_urls=https://should.always.be.used.instead.of.http.de/\nnumargs=42\nempty_argumt=\ntype=type01\nlong_argument_name_here=5\nanother_bool=0\nsilly_name_thing=UpperAndLower_Casess\nyet_anothernother=https://is.much.better.then.http.because.of.stuff/with/path/is/good/too/12345894/","and_an_ip4":"12.345.678.90","@version":"1","error_url_path":"/usr/local/application/with/a/lot/of/sub/directories/in/it.rs","logstash":"who.lines.logstash.probably.on","uuids->":"36ba15e0-d04f-4ae8-a6aa-0d3e65490d76","anotherfilename":"/usr/local/really/long/applicatio/name/that/leads/to/a/script","environment":"Development","floatasstr":"123.56","there_string:":"oh my what?","arry":["elemen","ts ","go in","her","e because I ","typ","e a","lot!!"],"message":"This is just a silly json to benchmark to emulate different sturcutres in reality!","argh":"ClbaulgV//Xer8lJ5jr10g==","oh_my_files":"/usr/local/there/are/way/too/many/file/names/in/this/json.txt","user_agent_os":"unknown os","error_host":"even.hosts.have.errors","application":"man","yam_message":"I can just keep writing here for all ethernity and bla bla bla bla it is just more","user_agent_browser":"unknown","error_url":"some.urls.dont.use.https/but/include/a/silly/path/so/we/can/include/them/here.jsonp","short_message":"ARE SHORT BECAUSE!!!","action":"give up.....","cakes!":"please","type":"chocol","log_level":"INFO","too_many_ho":"sts.oh.my.we.can.just.keepit","controller":"no_one_has_controll!","key_keykeykey":"key","a proper_timestamp_ja":"2018-07-23T12:19:16-04:00","and_yet_another":"host.with.lots.of.dots.in.it.gr","@timestamp":"2018-07-23T16:19:16.821Z","level":6}`;
// add tests
suite
  .add('wasm', function () {
    wasm.find_structural_indexes();
  })
  .add('json', function () {
    JSON.parse(data);
  })
  // add listeners
  .on('cycle', function (event) {
    console.log(String(event.target));
  })
  .on('complete', function () {
    console.log('Fastest is ' + this.filter('fastest').map('name'));
  })
  // run async
  .run({ async: true });
