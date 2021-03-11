/*=============================================================================
	UnNames.h: Header file registering global hardcoded Unreal names.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Macros.
-----------------------------------------------------------------------------*/

// Define a message as an enumeration.
#ifndef REGISTER_NAME
	#define REGISTER_NAME(num,name) NAME_##name = num,
	#define REG_NAME_HIGH(num,name) NAME_##name = num,
	#define REGISTERING_ENUM
	enum EName {
#endif

/*-----------------------------------------------------------------------------
	Hardcoded names which are not messages.
-----------------------------------------------------------------------------*/

// Special zero value, meaning no name
REG_NAME_HIGH(  0, None                         )

// Class property types; these map straight onto hardcoded property types
REGISTER_NAME(  1, ByteProperty                 )
REGISTER_NAME(  2, IntProperty                  )
REGISTER_NAME(  3, BoolProperty                 )
REGISTER_NAME(  4, FloatProperty                )
REGISTER_NAME(  5, ObjectProperty               )
REGISTER_NAME(  6, NameProperty                 )
REGISTER_NAME(  7, DelegateProperty             )
REGISTER_NAME(  8, ClassProperty                )
REGISTER_NAME(  9, ArrayProperty                )
REGISTER_NAME( 10, StructProperty               )
REGISTER_NAME( 11, VectorProperty               )
REGISTER_NAME( 12, RotatorProperty              )
REGISTER_NAME( 13, StrProperty                  )
REGISTER_NAME( 14, MapProperty                  )
REGISTER_NAME( 15, FixedArrayProperty           )

// Packages
REGISTER_NAME( 20, Core                         )
REGISTER_NAME( 21, Engine                       )
REGISTER_NAME( 22, Editor                       )
REGISTER_NAME( 23, Gameplay                     )

// UnrealScript types
REG_NAME_HIGH( 80, Byte                         )
REG_NAME_HIGH( 81, Int                          )
REG_NAME_HIGH( 82, Bool                         )
REG_NAME_HIGH( 83, Float                        )
REG_NAME_HIGH( 84, Name                         )
REG_NAME_HIGH( 85, String                       )
REG_NAME_HIGH( 86, Struct                       )
REG_NAME_HIGH( 87, Vector                       )
REG_NAME_HIGH( 88, Rotator                      )
REG_NAME_HIGH( 90, Color                        )
REG_NAME_HIGH( 91, Plane                        )
REG_NAME_HIGH( 92, Button                       )
REG_NAME_HIGH( 93, CompressedPosition           )

// Keywords
REGISTER_NAME(100, Begin                        )
REG_NAME_HIGH(102, State                        )
REG_NAME_HIGH(103, Function                     )
REG_NAME_HIGH(104, Self                         )
REG_NAME_HIGH(105, True                         )
REG_NAME_HIGH(106, False                        )
REG_NAME_HIGH(107, Transient                    )
REG_NAME_HIGH(108, RuntimeStatic                )
REG_NAME_HIGH(117, Enum                         )
REG_NAME_HIGH(119, Replication                  )
REG_NAME_HIGH(120, Reliable                     )
REG_NAME_HIGH(121, Unreliable                   )
REG_NAME_HIGH(122, Always                       )

// Object class names
REGISTER_NAME(150, Field                        )
REGISTER_NAME(151, Object                       )
REGISTER_NAME(152, TextBuffer                   )
REGISTER_NAME(153, Linker                       )
REGISTER_NAME(154, LinkerLoad                   )
REGISTER_NAME(155, LinkerSave                   )
REGISTER_NAME(156, Subsystem                    )
REGISTER_NAME(157, Factory                      )
REGISTER_NAME(158, TextBufferFactory            )
REGISTER_NAME(159, Exporter                     )
REGISTER_NAME(160, StackNode                    )
REGISTER_NAME(161, Property                     )
REGISTER_NAME(162, Camera                       )
REGISTER_NAME(163, PlayerInput                  )

// Creation and destruction
REGISTER_NAME(300, Spawned                      )
REGISTER_NAME(301, Destroyed                    )

// Gaining/losing actors
REGISTER_NAME(302, GainedChild                  )
REGISTER_NAME(303, LostChild                    )
REGISTER_NAME(304, Probe4                       )
REGISTER_NAME(305, Probe5                       )

// Triggers
REGISTER_NAME(306, Trigger                      )
REGISTER_NAME(307, UnTrigger                    )

// Physics & world interaction
REGISTER_NAME(308, Timer                        )
REGISTER_NAME(309, HitWall                      )
REGISTER_NAME(310, Falling                      )
REGISTER_NAME(311, Landed                       )
REGISTER_NAME(312, PhysicsVolumeChange          )
REGISTER_NAME(313, Touch                        )
REGISTER_NAME(314, UnTouch                      )
REGISTER_NAME(315, Bump                         )
REGISTER_NAME(316, BeginState                   )
REGISTER_NAME(317, EndState                     )
REGISTER_NAME(318, BaseChange                   )
REGISTER_NAME(319, Attach                       )
REGISTER_NAME(320, Detach                       )
REGISTER_NAME(321, ActorEntered                 )
REGISTER_NAME(322, ActorLeaving                 )
REGISTER_NAME(323, ZoneChange                   )
REGISTER_NAME(324, AnimEnd                      )
REGISTER_NAME(325, EndedRotation                )
REGISTER_NAME(326, InterpolateEnd               )
REGISTER_NAME(327, EncroachingOn                )
REGISTER_NAME(328, EncroachedBy                 )
REGISTER_NAME(329, NotifyTurningInPlace         )
REGISTER_NAME(330, HeadVolumeChange             )
REGISTER_NAME(331, PostTouch                    )
REGISTER_NAME(332, PawnEnteredVolume            )
REGISTER_NAME(333, MayFall                      )
REGISTER_NAME(334, CheckDirectionChange         )
REGISTER_NAME(335, PawnLeavingVolume            )

// Updates
REGISTER_NAME(336, Tick                         )
REGISTER_NAME(337, PlayerTick                   )
REGISTER_NAME(338, ModifyVelocity               )

// AI
REGISTER_NAME(339, CheckMovementTransition      )
REGISTER_NAME(340, SeePlayer                    )
REGISTER_NAME(342, HearNoise                    )
REGISTER_NAME(343, UpdateEyeHeight              )
REGISTER_NAME(344, SeeMonster                   )
REGISTER_NAME(347, BotDesireability             )
REGISTER_NAME(348, NotifyBump                   )
REGISTER_NAME(349, NotifyPhysicsVolumeChange    )
REGISTER_NAME(350, AIHearSound                  )
REGISTER_NAME(351, NotifyHeadVolumeChange       )
REGISTER_NAME(352, NotifyLanded                 )
REGISTER_NAME(353, NotifyHitWall                )
REGISTER_NAME(354, PostNetReceive               )
REGISTER_NAME(355, PreBeginPlay                 )
REGISTER_NAME(356, BeginPlay                    )
REGISTER_NAME(357, PostBeginPlay                )
REGISTER_NAME(358, PostLoadBeginPlay            )
REGISTER_NAME(359, PhysicsChangedFor            )
REGISTER_NAME(360, ActorEnteredVolume           )
REGISTER_NAME(361, ActorLeavingVolume           )
REGISTER_NAME(362, PrepareForMove               )

// Special tag meaning 'All probes'
REGISTER_NAME(363, All                          )

REGISTER_NAME(364, SceneStarted                 )
REGISTER_NAME(365, SceneEnded                   )

// Constants
REG_NAME_HIGH(600, Vect                         )
REG_NAME_HIGH(601, Rot                          )
REG_NAME_HIGH(605, ArrayCount                   )
REG_NAME_HIGH(606, EnumCount                    )
REG_NAME_HIGH(607, Rng                          )

// Flow control
REG_NAME_HIGH(620, Else                         )
REG_NAME_HIGH(621, If                           )
REG_NAME_HIGH(622, Goto                         )
REG_NAME_HIGH(623, Stop                         )
REG_NAME_HIGH(625, Until                        )
REG_NAME_HIGH(626, While                        )
REG_NAME_HIGH(627, Do                           )
REG_NAME_HIGH(628, Break                        )
REG_NAME_HIGH(629, For                          )
REG_NAME_HIGH(630, ForEach                      )
REG_NAME_HIGH(631, Assert                       )
REG_NAME_HIGH(632, Switch                       )
REG_NAME_HIGH(633, Case                         )
REG_NAME_HIGH(634, Default                      )
REG_NAME_HIGH(635, Continue                     )

// Variable overrides
REG_NAME_HIGH(640, Private                      )
REG_NAME_HIGH(641, Const                        )
REG_NAME_HIGH(642, Out                          )
REG_NAME_HIGH(643, Export                       )
REG_NAME_HIGH(644, EdFindable                   )
REG_NAME_HIGH(646, Skip                         )
REG_NAME_HIGH(647, Coerce                       )
REG_NAME_HIGH(648, Optional                     )
REG_NAME_HIGH(649, Input                        )
REG_NAME_HIGH(650, Config                       )
REG_NAME_HIGH(652, Travel                       )
REG_NAME_HIGH(653, EditConst                    )
REG_NAME_HIGH(654, Localized                    )
REG_NAME_HIGH(655, GlobalConfig                 )
REG_NAME_HIGH(656, SafeReplace                  )
REG_NAME_HIGH(657, New                          )
REG_NAME_HIGH(658, Protected                    )
REG_NAME_HIGH(659, Public                       )
REG_NAME_HIGH(660, EditInline                   )
REG_NAME_HIGH(661, EditInlineUse                )
REG_NAME_HIGH(662, Deprecated                   )
REG_NAME_HIGH(663, EditConstArray               )
REG_NAME_HIGH(664, EditInlineNotify             )
REG_NAME_HIGH(665, Automated                    )
REG_NAME_HIGH(666, PCOnly                       )
REG_NAME_HIGH(667, Align                        )
REG_NAME_HIGH(668, AutoLoad                     )

// Class overrides
REG_NAME_HIGH(671, Intrinsic                    )
REG_NAME_HIGH(672, Within                       )
REG_NAME_HIGH(673, Abstract                     )
REG_NAME_HIGH(674, Package                      )
REG_NAME_HIGH(675, Guid                         )
REG_NAME_HIGH(676, Parent                       )
REG_NAME_HIGH(677, Class                        )
REG_NAME_HIGH(678, Extends                      )
REG_NAME_HIGH(679, NoExport                     )
REG_NAME_HIGH(680, Placeable                    )
REG_NAME_HIGH(681, PerObjectConfig              )
REG_NAME_HIGH(682, NativeReplication            )
REG_NAME_HIGH(683, NotPlaceable                 )
REG_NAME_HIGH(684, EditInlineNew                )
REG_NAME_HIGH(685, NotEditInlineNew             )
REG_NAME_HIGH(686, HideCategories               )
REG_NAME_HIGH(687, ShowCategories               )
REG_NAME_HIGH(688, CollapseCategories           )
REG_NAME_HIGH(689, DontCollapseCategories       )

// State overrides
REG_NAME_HIGH(690, Auto                         )
REG_NAME_HIGH(691, Ignores                      )

// Auto-instanced subobjects
REG_NAME_HIGH(692, Instanced                    )

// Calling overrides
REG_NAME_HIGH(695, Global                       )
REG_NAME_HIGH(696, Super                        )
REG_NAME_HIGH(697, Outer                        )

// Class overrides
REG_NAME_HIGH(698, DependsOn                    )

// Function Overrides
REG_NAME_HIGH(700, Operator                     )
REG_NAME_HIGH(701, PreOperator                  )
REG_NAME_HIGH(702, PostOperator                 )
REG_NAME_HIGH(703, Final                        )
REG_NAME_HIGH(704, Iterator                     )
REG_NAME_HIGH(705, Latent                       )
REG_NAME_HIGH(706, Return                       )
REG_NAME_HIGH(707, Singular                     )
REG_NAME_HIGH(708, Simulated                    )
REG_NAME_HIGH(709, Exec                         )
REG_NAME_HIGH(710, Event                        )
REG_NAME_HIGH(711, Static                       )
REG_NAME_HIGH(712, Native                       )
REG_NAME_HIGH(713, Invariant                    )
REG_NAME_HIGH(714, Delegate                     )

// Variable overrides
REG_NAME_HIGH(720, Var                          )
REG_NAME_HIGH(721, Local                        )
REG_NAME_HIGH(722, Import                       )
REG_NAME_HIGH(723, From                         )

// Special commands
REG_NAME_HIGH(730, Spawn                        )
REG_NAME_HIGH(731, Array                        )
REG_NAME_HIGH(732, Map                          )

// Misc
REGISTER_NAME(740, Tag                          )
REGISTER_NAME(742, Role                         )
REGISTER_NAME(743, RemoteRole                   )
REGISTER_NAME(744, System                       )
REGISTER_NAME(745, User                         )

// Log messages
REGISTER_NAME(760, Log                          )
REGISTER_NAME(761, Critical                     )
REGISTER_NAME(762, Init                         )
REGISTER_NAME(763, Exit                         )
REGISTER_NAME(764, Cmd                          )
REGISTER_NAME(765, Play                         )
REGISTER_NAME(766, Console                      )
REGISTER_NAME(767, Warning                      )
REGISTER_NAME(768, ExecWarning                  )
REGISTER_NAME(769, ScriptWarning                )
REGISTER_NAME(770, ScriptLog                    )
REGISTER_NAME(771, Dev                          )
REGISTER_NAME(772, DevNet                       )
REGISTER_NAME(773, DevPath                      )
REGISTER_NAME(774, DevNetTraffic                )
REGISTER_NAME(775, DevAudio                     )
REGISTER_NAME(776, DevLoad                      )
REGISTER_NAME(777, DevSave                      )
REGISTER_NAME(778, DevGarbage                   )
REGISTER_NAME(779, DevKill                      )
REGISTER_NAME(780, DevReplace                   )
REGISTER_NAME(781, DevMusic                     )
REGISTER_NAME(782, DevSound                     )
REGISTER_NAME(783, DevCompile                   )
REGISTER_NAME(784, DevBind                      )
REGISTER_NAME(785, Localization                 )
REGISTER_NAME(786, Compatibility                )
REGISTER_NAME(787, NetComeGo                    )
REGISTER_NAME(788, Title                        )
REGISTER_NAME(789, Error                        )
REGISTER_NAME(790, Heading                      )
REGISTER_NAME(791, SubHeading                   )
REGISTER_NAME(792, FriendlyError                )
REGISTER_NAME(793, Progress                     )
REGISTER_NAME(794, UserPrompt                   )
REGISTER_NAME(795, AILog                        )

// Console text colors
REGISTER_NAME(800, White                        )
REGISTER_NAME(801, Black                        )
REGISTER_NAME(802, Red                          )
REGISTER_NAME(803, Green                        )
REGISTER_NAME(804, Blue                         )
REGISTER_NAME(805, Cyan                         )
REGISTER_NAME(806, Magenta                      )
REGISTER_NAME(807, Yellow                       )
REGISTER_NAME(808, DefaultColor                 )

// Misc
REGISTER_NAME(820, KeyType                      )
REGISTER_NAME(821, KeyEvent                     )
REGISTER_NAME(822, Write                        )
REGISTER_NAME(823, Message                      )
REGISTER_NAME(824, InitialState                 )
REGISTER_NAME(825, Texture                      )
REGISTER_NAME(826, Sound                        )
REGISTER_NAME(827, FireTexture                  )
REGISTER_NAME(828, IceTexture                   )
REGISTER_NAME(829, WaterTexture                 )
REGISTER_NAME(830, WaveTexture                  )
REGISTER_NAME(831, WetTexture                   )
REGISTER_NAME(832, Main                         )
REGISTER_NAME(833, NotifyLevelChange            )
REGISTER_NAME(834, VideoChange                  )
REGISTER_NAME(835, SendText                     )
REGISTER_NAME(836, SendBinary                   )
REGISTER_NAME(837, ConnectFailure               )
REGISTER_NAME(838, Length                       )
REGISTER_NAME(839, Insert                       )
REGISTER_NAME(840, Remove                       )
REGISTER_NAME(841, UTrace                       )
REGISTER_NAME(843, MenuWidgetBase               )
REGISTER_NAME(844, MenuSprite                   )
REGISTER_NAME(845, MenuText                     )
REGISTER_NAME(846, MenuDecoText                 )
REGISTER_NAME(847, MenuActiveWidget             )
REGISTER_NAME(848, MenuButton                   )
REGISTER_NAME(849, MenuButtonSprite             )
REGISTER_NAME(850, MenuButtonText               )
REGISTER_NAME(851, MenuButtonEnum               )
REGISTER_NAME(852, MenuCheckBox                 )
REGISTER_NAME(853, MenuCheckBoxSprite           )
REGISTER_NAME(854, MenuCheckBoxText             )
REGISTER_NAME(855, MenuEditBox                  )
REGISTER_NAME(856, MenuStringList               )
REGISTER_NAME(857, MenuDefault                  )
REGISTER_NAME(858, MenuBindingBox               )
REGISTER_NAME(859, MenuLayer                    )
REGISTER_NAME(860, MenuScrollBar                )
REGISTER_NAME(861, MenuActor                    )
REGISTER_NAME(862, MenuScrollArea               )
REGISTER_NAME(863, MenuProgressBar              )
REGISTER_NAME(864, MenuMovie                    )
REGISTER_NAME(865, CloseMenu                    )
REGISTER_NAME(866, HandleInputBack              )
REGISTER_NAME(867, DrawMenu                     )

// Animation sequence names
REGISTER_NAME(868, MeleeAttack                  )
REGISTER_NAME(869, MeleeAttack2                 )
REGISTER_NAME(870, Lunge                        )
REGISTER_NAME(871, Lunge2                       )
REGISTER_NAME(872, DodgeLeft                    )
REGISTER_NAME(873, DodgeRight                   )
REGISTER_NAME(874, DodgeDown                    )
REGISTER_NAME(875, StepLeft                     )
REGISTER_NAME(876, StepRight                    )
REGISTER_NAME(877, EmoteHearEnemy               )
REGISTER_NAME(878, EmoteSeeEnemy                )
REGISTER_NAME(879, CuriousToRelax               )
REGISTER_NAME(880, Shutdown                     )
REGISTER_NAME(881, Startup                      )
REGISTER_NAME(882, WallAttach                   )
REGISTER_NAME(883, WallDetach                   )
REGISTER_NAME(884, TakeOff                      )
REGISTER_NAME(885, Land                         )
REGISTER_NAME(886, ThrowGrenade                 )
REGISTER_NAME(887, FlyUp                        )
REGISTER_NAME(888, FlyDown                      )
REGISTER_NAME(889, FlyLeft                      )
REGISTER_NAME(890, FlyRight                     )
REGISTER_NAME(891, FlyForward                   )
REGISTER_NAME(892, FlyBackward                  )
REGISTER_NAME(893, spine1                       )
REGISTER_NAME(894, HealImmobile                 )
REGISTER_NAME(895, Executed                     )
REGISTER_NAME(896, Reload                       )
REGISTER_NAME(897, Dive                         )
REGISTER_NAME(898, BerserkAttack                )
REGISTER_NAME(899, BerserkAttack2               )
REGISTER_NAME(900, Fire                         )
REGISTER_NAME(901, Fire2                        )
REGISTER_NAME(902, LaunchMissiles               )
REGISTER_NAME(903, CrouchLeanLeft               )
REGISTER_NAME(904, CrouchLeanRight              )
REGISTER_NAME(905, LeanLeft                     )
REGISTER_NAME(906, LeanRight                    )
REGISTER_NAME(907, CeilingAttach                )
REGISTER_NAME(908, CeilingDetach                )
REGISTER_NAME(909, MeleeAttack3                 )
REGISTER_NAME(910, GrenadeReaction              )
REGISTER_NAME(911, SprintDive                   )
REGISTER_NAME(912, FireRockets                  )
REGISTER_NAME(913, Scripting                    )
REGISTER_NAME(914, WallStandCornerLeft          )
REGISTER_NAME(915, WallStandCornerRight         )
REGISTER_NAME(916, WallStandReturnLeft          )
REGISTER_NAME(917, WallStandReturnRight         )
REGISTER_NAME(918, BleedOut                     )
REGISTER_NAME(919, ForceName                    )

// Dummy name for ingame objects to avoid allocating tons of unique names
REGISTER_NAME(920, InGameTempName               )

// NoAutoLoad
REGISTER_NAME(921, NoAutoLoad                   )

// Animation sequence names
REGISTER_NAME(922, Load                         )
REGISTER_NAME(923, Holster                      )
REGISTER_NAME(924, EyeBlink                     )
REGISTER_NAME(925, EyeOpen                      )
REGISTER_NAME(926, EyeClosed                    )
REGISTER_NAME(927, eyelid                       )
REGISTER_NAME(928, EnterDefensiveBreathe        )
REGISTER_NAME(929, ExitDefensiveBreathe         )
REGISTER_NAME(930, DefensiveBreathe             )
REGISTER_NAME(931, ScanArea                     )
REGISTER_NAME(932, ProximityDefense             )
REGISTER_NAME(933, EnterStandRocketBreathe      )
REGISTER_NAME(934, ExitStandRocketBreathe       )
REGISTER_NAME(935, StandScanArea                )
REGISTER_NAME(936, StandRocketFire              )

/*-----------------------------------------------------------------------------
	Closing.
-----------------------------------------------------------------------------*/

#ifdef REGISTERING_ENUM
	};
	#undef REGISTER_NAME
	#undef REG_NAME_HIGH
	#undef REGISTERING_ENUM
#endif

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
